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
/* local private */
#include "connection.h"


int
connectionA(int argc, void *argv[]) {
    // struct chttpd *s = argv[0];
    // int fd = (long) argv[1];
    // union saddr *caddr = (union saddr *)argv[2];
    // int status;
    // int ret = 0;
    // struct http_request *req;
    // struct route *r;

    return -1;
    // req = http_request_new(fd, caddr);
    // if (req == NULL) {
    //     close(fd);
    //     return -1;
    // }

    // /* read as much as possible from the socket */
    // if (http_request_readallA(req) < 16) {
    //     /* less than minimum startline: "GET / HTTP/1.1"
    //      */
    //     ret = -1;
    //     goto done;
    // }

    // /* only parse the startline */
    // if (http_request_startline_parse(req)) {
    //     ret = -1;
    //     goto done;
    // }

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

// done:
//     http_request_close(req);
//     http_request_free(req);
//     close(fd);
//     return ret;
}
