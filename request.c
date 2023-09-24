// Copyright 2023 Vahid Mardani
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
#include <clog.h>
#include <mrb.h>
#include <caio.h>

#include "chttpd.h"
#include "connection.h"
#include "request.h"
#include "request_parser.h"
#include "response.h"
#include "route.h"


void
chttpd_request_free(struct chttpd_request *req) {
    if (req == NULL) {
        return;
    }

    if (req->header) {
        free(req->header);
    }

    memset(req, 0, sizeof(struct chttpd_request));
}


void
requestA(struct caio_task *self, struct chttpd_request *req) {
    char *header;
    ssize_t headerlen;
    CORO_START;
    int hsize = CHTTPD_HEADERSIZE + 4;

    if (req->status == CCS_REQUEST_HEADER) {
        /* Check the whole header is available or not */
        headerlen = mrb_search(req->inbuff, "\r\n\r\n", 4, 0, hsize);
        if (headerlen == -1) {
            headerlen = mrb_search(req->inbuff, "\n\n", 2, 0, hsize);
        }

        if (headerlen == -1) {
            // TODO: Preserve searched area to improve performance.
            if (mrb_used(req->inbuff) >= hsize) {
                req->status = CCS_CLOSING;
            }
            CORO_RETURN;
        }
        headerlen += 2;

        /* Allocate memory for request header */
        header = malloc(headerlen + 1);
        if (header == NULL) {
            req->status = CCS_CLOSING;
            free(header);
            CORO_RETURN;
        }

        /* Read the HTTP header from request buffer */
        if (mrb_get(req->inbuff, header, headerlen) != headerlen) {
            req->status = CCS_CLOSING;
            free(header);
            CORO_RETURN;
        }

        /* Parse the request */
        if (chttpd_request_parse(req, header, headerlen)) {
            /* Request parse error */
            req->status = CCS_CLOSING;
            free(header);
            CORO_RETURN;
        }

        /* Route(Find handler) */
        if (chttpd_route(req)) {
            chttpd_response(req, "404 Not Found");
            chttpd_connection_close(req);
            req->status = CCS_CLOSING;
            free(header);
            CORO_RETURN;
        }
    }

    if (req->handler == NULL) {
        chttpd_route(req);
    }
    // TODO: Find handler
    // TODO: Dispatch

    CORO_FINALLY;
    chttpd_request_free(req);
}
