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

/* local public */
#include "chttpd/chttpd.h"

/* local private */
#include "privatetypes.h"
#include "router.h"
#include "connection.h"


static int
_init(struct chttpd *s, struct chttpd_connection *c, int fd,
        union saddr *peer) {
    if (mrb_init(&c->ring, s->config->connectionbuffer_mempages)) {
        return -1;
    }

    c->request = chttp_request_new(s->config->requestbuffer_mempages);
    if (c->request == NULL) {
        mrb_deinit(&c->ring);
        return -1;
    }

    c->fd = fd;
    memcpy(&c->peer, peer, sizeof(union saddr));
    return 0;
}


static int
_free(struct chttpd_connection *c) {
    int ret = 0;

    ret |= mrb_deinit(&c->ring);
    ret |= chttp_request_free(c->request);
    return ret;
}


static int
_readA(struct chttpd_connection *c, size_t len) {
    ssize_t bytes;

    DEBUG("fd: %d", c->fd);
    pcaio_relaxA(0);

retry:
    bytes = mrb_readin(&c->ring, c->fd, len);
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


static ssize_t
_readallA(struct chttpd_connection *c) {
    size_t avail;

    avail = mrb_available(&c->ring);
    if ((avail) && _readA(c, avail) == -1) {
        return -1;
    }

    return mrb_used(&c->ring);
}


/** search inside the input ring buffer.
 * returns:
 * -1: not found
 * -2: buffer is full and not found.
 *  n: length of found string.
 */
static ssize_t
_ring_search(struct chttpd_connection *c, const char *s) {
    ssize_t ret;

    ret = mrb_search(&c->ring, s, strlen(s), 0, -1);
    if (ret == -1) {
        if (mrb_isfull(&c->ring)) {
            return -2;
        }

        return -1;
    }

    return ret;
}


int
connectionA(int argc, void *argv[]) {
    int ret = 0;
    struct chttpd *s = argv[0];
    int fd = (long) argv[1];
    union saddr *peer = (union saddr *)argv[2];
    struct chttpd_connection c;
    ssize_t headerlen;
    chttp_status_t status;
    struct route *route;

    INFO("new connection: %s, fd: %d",  saddr2a(peer), fd);
    if (_init(s, &c, fd, peer)) {
        return -1;
    }

    for (;;) {
        /* read as much as possible from the socket */
        if (_readallA(&c) < 16) {
            /* less than minimum startline: "GET / HTTP/1.1" */
            chttpd_responseA(&c, 400, NULL);
            ret = -1;
            break;
        }

        /* search for the end of the request's header */
        headerlen = _ring_search(&c, "\r\n\r\n");
        if (headerlen < 16) {
            chttpd_responseA(&c, 400, NULL);
            ret = -1;
            break;
        }

        headerlen += 2;
        status = chttp_request_parse(c.request, mrb_readerptr(&c.ring),
                headerlen);
        mrb_skip(&c.ring, headerlen);
        if (status > 0) {
            chttpd_responseA(&c, status, NULL);
            ret = -1;
            break;
        }

        if (status < 0) {
            ERROR("status: %d", status);
            ret = -1;
            break;
        }

        if (mrb_skip(&c.ring, headerlen)) {
            ERROR("mrb_skip");
            ret = -1;
            break;
        }

        route = router_find(&s->router, c.request->verb, c.request->path);
        if (route == NULL) {
            chttpd_responseA(&c, 404, NULL);
            continue;
        }

        INFO("new request: %s %s %s, route: %p",
                c.request->verb, c.request->path, c.request->query, route);
    }

    close(fd);
    if (_free(&c)) {
        ret = -1;
    }

    return ret;
}
//     // if (r->handler(req, r->ptr)) {
//     //     // TODO: log the unhandled server error
//     //     http_response_rejectA(req, 500, http_status_text(500));
//     // }
