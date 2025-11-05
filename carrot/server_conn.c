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
#include <stdio.h>
#include <errno.h>

/* thirdparty */
#include <clog.h>
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>

/* local public */
#include "carrot/server.h"

/* local private */
#include "common.h"
#include "router.h"


static int
_init(struct carrot_server *s, struct carrot_server_conn *c, int fd,
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
_reset(struct carrot_server_conn *c) {
    mrb_reset(&c->ring);
    chttp_request_reset(c->request);
}


static int
_free(struct carrot_server_conn *c) {
    int ret = 0;

    ret |= mrb_deinit(&c->ring);
    free(c->request);

    c->request = NULL;
    return ret;
}


// /** ensure atleast count bytes are exists in the connection's circular
//  * buffer and tries to read the missing bytes.
//  * returns:
//  * -2: if the count is greater than the buffer size.
//  * -1: if read error
//  *  0: if total numbers of bytes inside the buffer is less than or equals to
//  *     count.
//  */
// int
// carrot_server_conn_atleastA(struct carrot_server_conn *c, size_t count) {
//     if (count > c->ring.size) {
//         return -2;
//     }
//
//     if (mrb_used(&c->ring) >= count) {
//         return 0;
//     }
//
//     if (carrot_server_conn_readallA(c, NULL) <= 0) {
//         return -1;
//     }
//
//     return 0;
// }


// /** search inside the input ring buffer.
//  * returns:
//  * -1: not found
//  *  n: length of found string.
//  */
// ssize_t
// carrot_server_conn_search(struct carrot_server_conn *c, const char *s) {
//     ssize_t ret;
//
//     ret = mrb_search(&c->ring, s, strlen(s), 0, -1);
//     if (ret == -1) {
//         return -1;
//     }
//
//     return ret;
// }


int
carrot_server_connA(int argc, void *argv[]) {
    int ret = 0;
    struct carrot_server *s = argv[0];
    int fd = (long) argv[1];
    union saddr *peer = (union saddr *)argv[2];
    struct carrot_server_conn c;
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
        _reset(&c);
    }

    close(fd);
    if (_free(&c)) {
        ret = -1;
    }

    return ret;
}
