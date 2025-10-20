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
#include "privatetypes.h"
#include "connection.h"

/* local public */
#include "chttpd/chttpd.h"



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
_deinit(struct chttpd_connection *c) {
    int ret = 0;

    ret |= mrb_deinit(&c->ring);
    ret |= chttp_request_free(c->request);
    return ret;
}


int
connectionA(int argc, void *argv[]) {
    struct chttpd *s = argv[0];
    int fd = (long) argv[1];
    union saddr *peer = (union saddr *)argv[2];
    struct chttpd_connection c;

    INFO("new connection: %s, fd: %d",  saddr2a(peer), fd);
    if (_init(s, &c, fd, peer)) {
        return -1;
    }


    if (_deinit(&c)) {
        return -1;
    }
    return 0;
}


//     struct chttpd_connection *c;
//     int ret = 0;
//     int seplen;
//     int headerlen;
//     chttp_status_t status;
//     // struct route *r;
//
//     c = connection_new(fd, peer);
//     if (c == NULL) {
//         return -1;
//     }
//
//     for (;;) {
//         /* read as much as possible from the socket */
//         if (connection_readallA(c) < 16) {
//             /* less than minimum startline: "GET / HTTP/1.1"
//              */
//             chttpd_rejectA(&c->req, 400, NULL);
//             // connection_ring_reset(&c);
//             continue;
//         }
//
//         headerlen = connection_ring_search(c, "\r\n\r\n");
//         if (headerlen < 0) {
//             headerlen = connection_ring_search(c, "\n\n");
//             if (headerlen >= 0) {
//                 seplen = 2;
//             }
//         }
//         else {
//             seplen = 4;
//         }
//
//         if (headerlen == -1) {
//             continue;
//         }
//
//         if (headerlen == -2) {
//             /* buffer is full and expression not found */
//             ret = -1;
//             break;
//         }
//
//         if (headerlen < 14) {
//             ret = -1;
//             goto done;
//         }
//
//         status = chttp_request_frombuffer(&c->req, mrb_readerptr(c->ring),
//                 headerlen + seplen);
//         mrb_skip(c->ring, headerlen + seplen);
//         if (status > 0) {
//             chttpd_rejectA(&c->req, status, NULL);
//             continue;
//         }
//
//         if (status) {
//             ERROR("status: %d", status);
//             ret = -1;
//             goto done;
//         }
//
//         // /* find handler */
//         // if (_findroute(&c->req, &r)) {
//         //     http_response_rejectA(req, 404, http_status_text(404));
//         //     goto done;
//         // }
//
//         INFO("new request: %s %s %s", c->req.verb, c->req.path, c->req.query);
//     }
//
// done:
//     close(fd);
//     connection_free(c);
//     return ret;
//
//     // INFO("new request: %s, fd: %d, %s %s",  saddr2a(caddr), fd,
//     //         req->verb, req->path);
//
//     // /* parse headers */
//     // status = http_request_header_parse(req);
//     // if (status == -1) {
//     //     ret = -1;
//     //     goto done;
//     // }
//     // if (status > 0) {
//     //     http_response_rejectA(req, status, http_status_text(status));
//     //     goto done;
//     // }
//
//     // if (r->handler(req, r->ptr)) {
//     //     // TODO: log the unhandled server error
//     //     http_response_rejectA(req, 500, http_status_text(500));
//     // }
// }
// int
// connection_readA(struct chttpd_connection *c, size_t len) {
//     ssize_t bytes;
//
//     pcaio_relaxA(0);
//
// retry:
//     bytes = mrb_readin(c->ring, c->fd, len);
//     if (bytes == -1) {
//         if (!RETRY(errno)) {
//             return -1;
//         }
//
//         if (pcaio_modio_await(c->fd, IOIN)) {
//             return -1;
//         }
//
//         goto retry;
//     }
//
//     errno = 0;
//     return bytes;
// }
//
//
// ssize_t
// connection_readallA(struct chttpd_connection *c) {
//     size_t avail;
//
//     avail = mrb_available(c->ring);
//     if ((avail) && connection_readA(c, avail) == -1) {
//         return -1;
//     }
//
//     return mrb_used(c->ring);
// }
//
//
// /** search inside the input ring buffer.
//  * returns:
//  * -1: not found
//  * -2: buffer is full and not found.
//  *  n: length of found string.
//  */
// int
// connection_ring_search(struct chttpd_connection *c, const char *s) {
//     ssize_t ret;
//
//     ret = mrb_search(c->ring, s, strlen(s), 0, -1);
//     if (ret == -1) {
//         if (mrb_isfull(c->ring)) {
//             return -2;
//         }
//
//         return -1;
//     }
//
//     return ret;
// }
//
//
// int
// connection_ring_reset(struct chttpd_connection *c, const char *s) {
//     // TODO: implement
//     return -1;
// }
