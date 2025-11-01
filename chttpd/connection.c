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


static void
_reset(struct chttpd_connection *c) {
    mrb_reset(&c->ring);
    chttp_request_reset(c->request);
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


/** read as much as possible from the peer into the connection's circular
 * buffer.
 * returns -2 if buffer full, -1 if read error and total numbers of bytes
 * inside the buffer on successful read.
 */
ssize_t
chttpd_connection_readallA(struct chttpd_connection *c) {
    size_t avail;

    avail = mrb_available(&c->ring);
    if (!avail) {
        return -2;
    }

    if (_readA(c, avail) == -1) {
        return -1;
    }

    return mrb_used(&c->ring);
}


/** ensure atleast count bytes are exists in the connection's circular
 * buffer and tries to read the missing bytes.
 * returns:
 * -2: if the count is greater than the buffer size.
 * -1: if read error
 *  0: if total numbers of bytes inside the buffer is less than or equals to
 *     count.
 */
int
chttpd_connection_atleastA(struct chttpd_connection *c, size_t count) {
    size_t used;
    ssize_t ret;

    if (count > c->ring.size) {
        return -2;
    }

    used = mrb_used(&c->ring);
    if (used >= count) {
        return 0;
    }

    ret = _readA(c, count - used);
    if (ret <= 0) {
        return -1;
    }

    return 0;
}


/** search inside the input ring buffer.
 * returns:
 * -1: not found
 *  n: length of found string.
 */
ssize_t
chttpd_connection_search(struct chttpd_connection *c, const char *s) {
    ssize_t ret;

    ret = mrb_search(&c->ring, s, strlen(s), 0, -1);
    if (ret == -1) {
        return -1;
    }

    return ret;
}


/** ensure atleast bytes are exists in the connection circular buffer and
 * search inside the circular buffer for the given expression.
 * returns:
 * -1: read error
 * -2: atleast is less than the buffer size.
 * -3: atleast N chars are exists in the buffer and not found.
 *  n: length of found string.
 */
ssize_t
chttpd_connection_readsearchA(struct chttpd_connection *c, const char *s,
        size_t atleast) {
    ssize_t readret;
    ssize_t searchret;

    readret = chttpd_connection_atleastA(c, atleast);
    if (readret) {
        return readret;
    }

    searchret = chttpd_connection_search(c, s);
    if (searchret == -1) {
        return -3;
    }

    return searchret;
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
        /* FIXME: check if this is a head-only request */
        headerlen = chttpd_connection_readsearchA(&c, "\r\n\r\n", 18);
        if (headerlen == -1) {
            /* connection error */
            ret = -1;
            break;
        }

        if (headerlen <= 0) {
            chttpd_response_errorA(&c, 400, NULL);
            ret = -1;
            break;
        }

        headerlen += 2;
        status = chttp_request_parse(c.request, mrb_readerptr(&c.ring),
                headerlen);
        if (status > 0) {
            chttpd_response_errorA(&c, status, NULL);
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
            chttpd_response_errorA(&c, 404, NULL);
            continue;
        }

        INFO("new request: %s %s %s, route: %p",
                c.request->verb, c.request->path, c.request->query, route);

        if (route->handler(&c, route->ptr)) {
            // TODO: log the unhandled server error
            chttpd_response_errorA(&c, 500, NULL);
            ret = -1;
            break;
        }

        /* make everything fresh for the next request */
        _reset(&c);
    }

    close(fd);
    if (_free(&c)) {
        ret = -1;
    }

    return ret;
}
