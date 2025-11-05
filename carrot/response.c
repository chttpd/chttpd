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
#include <stdarg.h>

/* thirdparty */
#include <clog.h>
#include <chttp/chttp.h>
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>

/* local public */
#include "carrot/carrot.h"

/* local private */
#include "common.h"


ssize_t
carrot_responseA(struct carrot_connection *c, int status, const char *text,
        const char *content, size_t contentlen) {
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
    ret = carrot_connection_sendpacket(c, &p);
    chttp_packet_free(&p);

    return ret;
}


ssize_t
carrot_response_errorA(struct carrot_connection *c, int status,
        const char *text) {
    const char *content;

    content = chttp_status_text(status);
    return carrot_responseA(c, status, text, content, strlen(content) + 1);
}


// ssize_t
// carrot_response_header_flushA(struct carrot_connection *c) {
//     struct chttp_packet *resp = &c->request->response;
//     size_t written;
//
//     written = writeA(c->fd, (void *)resp->header, resp->headerlen);
//     if (written != resp->headerlen) {
//         // TODO: write the rest of the buffer later after pcaio_relaxA
//         return -1;
//     }
//
//     return written;
// }
//
//
// ssize_t
// carrot_response_content_flushA(struct carrot_connection *c) {
//     struct chttp_packet *resp = &c->request->response;
//     size_t written;
//
//     written = writeA(c->fd, resp->content, resp->contentlen);
//     if (written != resp->contentlen) {
//         // TODO: write the rest of the buffer later after pcaio_relaxA
//         return -1;
//     }
//
//     resp->contentlen = 0;
//     return written;
// }
//
//
// ssize_t
// carrot_response_flushchunkA(struct carrot_connection *c) {
//     struct chttp_packet *resp = &c->request->response;
//     ssize_t len;
//
//     len = carrot_response_writechunkA(c, resp->content, resp->contentlen);
//     if (len != resp->contentlen) {
//         // TODO: write the rest of the buffer later after pcaio_relaxA
//         return -1;
//     }
//
//     resp->contentlen = 0;
//     return len;
// }
//
//
// int
// carrot_response_chunk_end(struct carrot_connection *c) {
//     if (writeA(c->fd, "0\r\n\r\n", 5) != 5) {
//         return -1;
//     }
//
//     return 0;
// }
