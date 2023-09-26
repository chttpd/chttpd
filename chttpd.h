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
#ifndef CHTTPD_H_
#define CHTTPD_H_


#include <sys/socket.h>

#include <mrb.h>
#include <caio.h>


#ifndef CHTTPD_HEADERSIZE
#define CHTTPD_HEADERSIZE 8192
#endif


#ifndef CHTTPD_REQUESTHEADERS_MAX
#define CHTTPD_REQUESTHEADERS_MAX 64
#endif


enum chttpd_request_status {
    CCS_REQUEST_HEADER,
    CCS_REQUEST_BODY,
    CCS_RESPONSE_HEADER,
    CCS_RESPONSE_BODY,
    CCS_CLOSING,
};


struct chttpd_request {
    enum chttpd_request_status status;

    /* Connection */
    int fd;
    struct sockaddr localaddr;
    struct sockaddr remoteaddr;
    mrb_t inbuff;
    mrb_t outbuff;

    /* Header buffer */
    char *header;
    size_t headerlen;

    /* HTTP headers */
    const char *headers[CHTTPD_REQUESTHEADERS_MAX];
    unsigned char headerscount;

    /* Attributes */
    const char *verb;
    const char *path;
    const char *version;
    const char *connection;
    const char *contenttype;
    int contentlength;

    /* Handler */
    caio_coro handler;
};


/* Router entry */
struct chttpd_route {
    const char *pattern;
    const char *verb;
    caio_coro handler;
};


/* chttpd state */
struct chttpd {
    /* Socket */
    const char *bindaddr;
    unsigned short bindport;

    /* Limits */
    int backlog;
    size_t buffsize;
    size_t maxconn;

    /* Routes */
    struct chttpd_route *routes;
};


/* Helper Macros */
#define CHTTPD_ROUTE(p, v, h) {(p), (v), (caio_coro)h}
#define CHTTPD_RESPONSE_FLUSH(req) while (chttpd_response_flush(req)) { \
        if (CORO_MUSTWAITFD()) { \
            CORO_WAITFD((req)->fd, CAIO_OUT); \
            continue; \
        } \
        chttpd_response_close(req); \
    }


#define CHTTPD_RESPONSE_FINALIZE(req) \
    chttpd_response_finalize(req); \
    CHTTPD_RESPONSE_FLUSH(req)


ASYNC
chttpdA(struct caio_task *self, struct chttpd *state);


int
chttpd_response_start(struct chttpd_request *req, const char *format, ...);


int
chttpd_response_header(struct chttpd_request *req, const char *format, ...);


int
chttpd_response_flush(struct chttpd_request *req);


int
chttpd_response_close(struct chttpd_request *req);


int
chttpd_response_finalize(struct chttpd_request *req);


int
chttpd_response_body(struct chttpd_request *req, const char *format, ...);


int
chttpd_forever(struct chttpd *restrict state);


#endif  // CHTTPD_H_
