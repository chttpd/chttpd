// Copyright 2025 Vahid Mardani
/*
 * This file is part of chttpd.
 *  chttpd is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  chttpd is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with chttpd. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Author: Vahid Mardani <vahid.mardani@gmail.com>
 */
/* standard */
#include <stdio.h>
#include <errno.h>

/* thirdparty */
#include <clog.h>
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>

/* local private */
#include "config.h"
#include "connection.h"


int
connection_readA(struct connection *c, size_t len) {
    ssize_t bytes;

    pcaio_relaxA(0);

retry:
    bytes = mrb_readin(c->ring, c->fd, len);
    if (bytes == -1) {
        if (!RETRY(errno)) {
            return -1;
        }

        if (pcaio_modio_await(c->fd, IOIN)) {
            return -1;
        }

        goto retry;
    }

    errno = 0;
    return bytes;
}


ssize_t
connection_readallA(struct connection *c) {
    size_t avail;

    avail = mrb_available(c->ring);
    if ((avail) && connection_readA(c, avail) == -1) {
        return -1;
    }

    return mrb_used(c->ring);
}


struct connection *
connection_new(int fd, union saddr *peer) {
    struct connection *c;

    c = malloc(sizeof(struct connection));
    if (c == NULL) {
        return NULL;
    }

    c->ring = mrb_create(CONFIG_CHTTPD_REQUEST_RINGSIZE);
    if (c->ring == NULL) {
        free(c);
        return NULL;
    }

    c->fd = fd;
    memcpy(&c->peeraddr, peer, sizeof(union saddr));
    return c;
}


int
connection_free(struct connection *c) {
    if (mrb_destroy(c->ring)) {
        return -1;
    }
    free(c);
    return 0;
}


int
connection_ring_search(struct connection *c, const char *s) {
    ssize_t ret;

    ret = mrb_search(c->ring, s, strlen(s), 0, -1);
    if (ret == -1) {
        if (mrb_isfull(c->ring)) {
            return -2;
        }

        return -1;
    }

    return ret;
}


int
connectionA(int argc, void *argv[]) {
    struct connection *c;
    // struct chttpd *s = argv[0];
    int fd = (long) argv[1];
    union saddr *caddr = (union saddr *)argv[2];
    int ret = 0;
    int seplen;
    int headerlen;
    httpstatus_t status;
    // struct route *r;

    INFO("new connection: %s, fd: %d",  saddr2a(caddr), fd);
    c = connection_new(fd, caddr);
    if (c == NULL) {
        return -1;
    }

    for (;;) {
        /* read as much as possible from the socket */
        if (connection_readallA(c) < 16) {
            /* less than minimum startline: "GET / HTTP/1.1"
             */
            continue;
        }

        headerlen = connection_ring_search(c, "\r\n\r\n");
        if (headerlen < 0) {
            headerlen = connection_ring_search(c, "\n\n");
            if (headerlen >= 0) {
                seplen = 2;
            }
        }
        else {
            seplen = 4;
        }

        if (headerlen == -1) {
            continue;
        }
        if (headerlen == -2) {
            /* buffer is full and expression not found */
            ret = -1;
            break;
        }

        if (headerlen < 14) {
            ret = -1;
            goto done;
        }

        DEBUG("Header found(%dB): %.*s", headerlen, headerlen,
                mrb_readerptr(c->ring));

        status = request_frombuffer(&c->req, mrb_readerptr(c->ring),
                headerlen + seplen);
        if (status) {
            ERROR("status: %d", status);
            ret = -1;
            goto done;
        }
        INFO("new request: %s %s %s", c->req.verb, c->req.path, c->req.query);
        mrb_skip(c->ring, headerlen + seplen);
    }

done:
    close(fd);
    connection_free(c);
    return ret;

    // INFO("new request: %s, fd: %d, %s %s",  saddr2a(caddr), fd,
    //         req->verb, req->path);

    // /* find handler */
    // if (_findroute(req, &r)) {
    //     http_response_rejectA(req, 404, http_status_text(404));
    //     goto done;
    // }

    // /* parse headers */
    // status = http_request_header_parse(req);
    // if (status == -1) {
    //     ret = -1;
    //     goto done;
    // }
    // if (status > 0) {
    //     http_response_rejectA(req, status, http_status_text(status));
    //     goto done;
    // }

    // if (r->handler(req, r->ptr)) {
    //     // TODO: log the unhandled server error
    //     http_response_rejectA(req, 500, http_status_text(500));
    // }
}
