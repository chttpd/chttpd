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


ssize_t
carrot_server_responseA(struct carrot_connection *c, int status,
        const char *text, const char *content, size_t contentlen, int flags) {
    struct chttp_packet p;
    ssize_t ret;

    if (text == NULL) {
        text = chttp_status_text(status);
    }

    if (contentlen == -1) {
        contentlen = strlen(content);
    }

    ERR(chttp_packet_allocate(&p, 1, 1, CHTTP_TE_NONE));
    ERR(chttp_packet_startresponse(&p, status, text));
    ERR(chttp_packet_contenttype(&p, "text/plain", "utf-8"));
    ERR(chttp_packet_write(&p, content, contentlen));
    if (flags & CARROT_SRF_APPENDCRLF) {
        ERR(chttp_packet_write(&p, "\r\n", 2));
    }
    ERR(chttp_packet_close(&p));
    ret = carrot_connection_sendpacketA(c, &p);
    chttp_packet_free(&p);

    return ret;
}


ssize_t
carrot_server_rejectA(struct carrot_connection *c, int status,
        const char *text) {
    const char *content;

    content = chttp_status_text(status);
    return carrot_server_responseA(c, status, text, content, -1,
            CARROT_SRF_APPENDCRLF);
}


int
server_connA(struct carrot_server *s, int fd, union saddr *peer) {
    int ret = 0;
    struct carrot_connection c;
    ssize_t headerlen;
    chttp_status_t status;
    struct route *route;
    char tmp[32];

    /* render the peer address for logging purpose */
    ERR(saddr_tostr(tmp, sizeof(tmp), peer));
    INFO("new connection: %s, fd: %d", tmp, fd);

    ERR(mrb_init(&c.ring, s->config->connectionbuffer_mempages));
    c.fd = fd;
    c.request = chttp_request_new(s->config->requestbuffer_mempages);
    if (c.request == NULL) {
        mrb_deinit(&c.ring);
        return -1;
    }

    /* preserve peer address */
    c.peer = *peer;

    /* connection main loop */
    for (;;) {
        /* read as much as possible from the socket */
        /* FIXME: check if this is a head-only request */
        headerlen = carrot_connection_recvsearchA(&c, "\r\n\r\n");
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
carrot_serverA(struct carrot_server *s) {
    union saddr listenaddr;
    struct sockaddr_storage caddrbuff;
    union saddr *caddr = (union saddr *)&caddrbuff;
    socklen_t socklen;
    int cfd;
    char tmp[32];

    s->listenfd = socket_bind(s->config->bind, &listenaddr);
    if (s->listenfd == -1) {
        return -1;
    }

    /* listen */
    if (listen(s->listenfd, s->config->backlog)) {
        return -1;
    }

    ERR(saddr_tostr(tmp, sizeof(tmp), &listenaddr));
    INFO("listening on: %s", tmp);
    for (;;) {
        socklen = sizeof(caddrbuff);
        cfd = accept4A(s->listenfd, (struct sockaddr*)&caddr, &socklen,
                SOCK_NONBLOCK);
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

        pcaio_fschedule(server_connA, NULL, 3, s, cfd, caddr);
    }

    // TODO: move it to pcaio task disposation callback
    close(s->listenfd);
    return 0;
}


int
carrot_server_main(struct carrot_server *s) {
    int ret;
    pcaio_task_t task;
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
