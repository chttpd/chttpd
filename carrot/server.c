// Copyright 2025 Vahid Mardani
/*
 * This file is part of carrot.
 *  carrot is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  carrot is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with carrot. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Author: Vahid Mardani <vahid.mardani@gmail.com>
 */
/* standard */
#include <stddef.h>
#include <stdio.h>
#include <errno.h>

/* thirdparty */
#include <clog.h>
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>
#include <pcaio/modepoll.h>

/* local public */
#include "carrot/server.h"
#include "carrot/addr.h"

/* local private */
#include "common.h"
#include "config.h"
#include "socket.h"
#include "router.h"
#include "server.h"


const struct carrot_server_config carrot_server_defaultconfig = {
    .bind = "127.0.0.1:8080",
    .backlog = 10,
    .requestbuffer_mempages = 1,
    .connectionbuffer_mempages = 1,
    .connections_max = 10,
};


void
carrot_server_makedefaults(struct carrot_server_config *c) {
    memcpy(c, &carrot_server_defaultconfig,
            sizeof(carrot_server_defaultconfig));
}


struct carrot_server *
carrot_server_new(const struct carrot_server_config *c) {
    struct carrot_server *s;

    s = malloc(sizeof(struct carrot_server));
    if (s == NULL) {
        return NULL;
    }

    s->listenfd = -1;
    s->router.count = 0;
    s->config = c;
    return s;
}


void
carrot_server_free(struct carrot_server *s) {
    if (s == NULL) {
        return;
    }
    free(s);
}


int
carrot_server_route(struct carrot_server *s, const char *verb, const char *path,
        carrot_handler_t handler, void *ptr) {
    return router_append(&s->router, verb, path, handler, ptr);
}


/** read as much as possible from the peer and returns length of the newly
 * read data, -2 when buffer is full, 0 on end-of-file and -1 on error.
 * The out ptr will set to the start of the received data on successfull read.
 */
int
carrot_server_recvallA(struct carrot_conn *c, char **out) {
    ssize_t bytes;
    char *start = mrb_readerptr(&c->ring);
    pcaio_relaxA(0);

retry:
    bytes = mrb_readallin(&c->ring, c->fd);
    if (bytes == -2) {
        return -2;
    }

    if (bytes == -1) {
        if (!RETRY(errno)) {
            return -1;
        }

        if (pcaio_modio_await(c->fd, IOIN)) {
            return -1;
        }

        errno = 0;
        goto retry;
    }

    if (out) {
        *out = start;
    }
    return bytes;
}


/**
 * return the number of characters which would have been written to the target
 * buffer and -2 if enought space had been available. otherwise, chunksize and
 * -1 for error.
 */
ssize_t
carrot_server_recvchunkA(struct carrot_conn *c, const char **start) {
    ssize_t chunksize;
    size_t garbage;
    char *in = mrb_readerptr(&c->ring);
    size_t inlen = mrb_used(&c->ring);
    ssize_t ret;

retry:
    chunksize = chttp_chunked_parse(in, inlen, start, &garbage);
    if (chunksize == 0) {
        return 0;
    }

    if (chunksize == -1) {
        return -1;
    }

    if (chunksize == -2) {
        /* more data needed */
        ret = carrot_server_recvallA(c, NULL);
        if (ret <= 0) {
            return  ret;
        }

        inlen += ret;
        goto retry;
    }

    mrb_skip(&c->ring, garbage);
    return chunksize;
}


ssize_t
carrot_server_responseA(struct carrot_conn *c, int status,
        const char *text, const char *content, size_t contentlen) {
    struct chttp_packet p;
    ssize_t ret;

    if (text == NULL) {
        text = chttp_status_text(status);
    }

    ERR(chttp_packet_allocate(&p, 1, 1, CHTTP_TE_NONE));
    ERR(chttp_packet_startresponse(&p, status, text));
    ERR(chttp_packet_contenttype(&p, "text/plain", "utf-8"));
    ERR(chttp_packet_write(&p, content, contentlen));
    ERR(chttp_packet_close(&p));
    ret = carrot_server_sendpacketA(c, &p);
    chttp_packet_free(&p);

    return ret;
}


ssize_t
carrot_server_sendpacketA(struct carrot_conn *c,
        struct chttp_packet *p) {
    struct iovec v[4];
    int vcount = sizeof(v) / sizeof(struct iovec);
    size_t totallen;
    size_t written;

    totallen = chttp_packet_iovec(p, v, &vcount);
    written = writevA(c->fd, v, vcount);
    if (written != totallen) {
        // TODO: write the rest of the buffer later after pcaio_relaxA
        return -1;
    }

    chttp_packet_reset(p);
    return totallen;
}


ssize_t
carrot_server_rejectA(struct carrot_conn *c, int status,
        const char *text) {
    const char *content;

    content = chttp_status_text(status);
    return carrot_server_responseA(c, status, text, content,
            strlen(content) + 1);
}


/** wait until read error, buffer become full or find the s inside the input
 * buffer.
 * search inside the circular buffer for the given expression.
 * returns:
 *  0: EOF and not found
 * -1: read error
 * -2: input buffer full and not found
 * -3: s is empty or NULL
 *  n: length of found string.
 */
ssize_t
carrot_server_recvsearchA(struct carrot_conn *c, const char *s) {
    size_t avail;
    char *chunk = mrb_readerptr(&c->ring);
    ssize_t chunksize = mrb_used(&c->ring);
    int slen;
    char *found;
    int lookbehind;

    if (s == NULL) {
        return -3;
    }

    slen = strlen(s);
    if (slen == 0) {
        return -3;
    }

    if (chunksize) {
        found = memmem(chunk, chunksize, s, slen);
        if (found) {
            return found - chunk;
        }
    }

    while ((avail = mrb_available(&c->ring))) {
        lookbehind = MIN(mrb_used(&c->ring), slen - 1);
        chunksize = carrot_server_recvallA(c, &chunk);
        if (chunksize < 0) {
            return chunksize;
        }

        if (chunksize && (chunksize < slen)) {
            continue;
        }

        found = memmem(chunk - lookbehind, chunksize, s, slen);
        if (found) {
            return found - mrb_readerptr(&c->ring);
        }

        if (chunksize == 0) {
            return 0;
        }
    }

    return -2;
}


int
carrot_server_connA(int argc, void *argv[]) {
    int ret = 0;
    struct carrot_server *s = argv[0];
    int fd = (long) argv[1];
    union saddr *peer = (union saddr *)argv[2];
    struct carrot_conn c;
    ssize_t headerlen;
    chttp_status_t status;
    struct route *route;

    INFO("new connection: %s, fd: %d",  saddr2a(peer), fd);
    ERR(mrb_init(&c.ring, s->config->connectionbuffer_mempages));
    c.fd = fd;
    c.request = chttp_request_new(s->config->requestbuffer_mempages);
    if (c.request == NULL) {
        mrb_deinit(&c.ring);
        return -1;
    }

    /* preserve peer address */
    memcpy(&c.peer, peer, sizeof(union saddr));

    /* connection main loop */
    for (;;) {
        /* read as much as possible from the socket */
        /* FIXME: check if this is a head-only request */
        headerlen = carrot_server_recvsearchA(&c, "\r\n\r\n");
        if (headerlen <= 0) {
            /* connection error */
            ret = -1;
            break;
        }

        headerlen += 2;
        status = chttp_request_parse(c.request, mrb_readerptr(&c.ring),
                headerlen);
        if (status > 0) {
            carrot_server_rejectA(&c, status, NULL);
            ret = -1;
            break;
        }

        if (status < 0) {
            ERROR("status: %d", status);
            ret = -1;
            break;
        }

        if (mrb_skip(&c.ring, headerlen + 2)) {
            ERROR("mrb_skip");
            ret = -1;
            break;
        }

        route = router_find(&s->router, c.request->verb, c.request->path);
        if (route == NULL) {
            carrot_server_rejectA(&c, 404, NULL);
            continue;
        }

        INFO("new request: %s %s %s, route: %p", c.request->verb,
                c.request->path, c.request->query, route);

        if (route->handler(&c, route->ptr)) {
            // TODO: log the unhandled server error
            carrot_server_rejectA(&c, 500, NULL);
            ret = -1;
            break;
        }

        /* make everything fresh for the next request */
        mrb_reset(&c.ring);
        chttp_request_reset(c.request);
    }

    /* free */
    close(fd);
    mrb_deinit(&c.ring);
    free(c.request);
    return ret;
}


int
carrot_serverA(int argc, void *argv[]) {
    struct carrot_server *s = (struct carrot_server *) argv[0];
    union saddr listenaddr;
    struct sockaddr_storage caddrbuff;
    union saddr *caddr = (union saddr *)&caddrbuff;
    socklen_t socklen;
    int cfd;

    s->listenfd = socket_bind(s->config->bind, &listenaddr);
    if (s->listenfd == -1) {
        return -1;
    }

    /* listen */
    if (listen(s->listenfd, s->config->backlog)) {
        return -1;
    }

    INFO("listening on: %s", saddr2a(&listenaddr));
    for (;;) {
        socklen = sizeof(caddrbuff);
        cfd = accept4A(s->listenfd, &caddr->sa, &socklen, SOCK_NONBLOCK);
        if (cfd == -1) {
            if (errno == ECONNABORTED) {
                /* ignore and continue */
                continue;
            }

            if ((errno == ENFILE) || (errno == EMFILE)) {
                /* open files limit, retry */
                WARN("open files linit reached");
                continue;
            }

            return -1;
        }

        pcaio_fschedule(carrot_server_connA, NULL, 3, s, cfd, caddr);
    }

    // TODO: move it to pcaio task disposation callback
    close(s->listenfd);
    return 0;
}


int
carrot_server_main(struct carrot_server *s) {
    int ret;
    struct pcaio_task *task;
    struct pcaio_iomodule *modepoll;

    /* register modules and tasks */
    if (pcaio_modepoll_use(16, &modepoll)) {
        return -1;
    }

    if (pcaio_modio_use(modepoll)) {
        return -1;
    }

    // TODO: task status
    task = pcaio_task_new(carrot_serverA, NULL, 1, s);
    if (task == NULL) {
        return -1;
    }

    /* run event loop */
    // TODO: only reflect the pcaio function result, and return task status
    //       instead
    ret = pcaio(1, &task, 1);
    carrot_server_free(s);

    return ret;
}
