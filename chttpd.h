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


#include <regex.h>
#include <sys/socket.h>

#include <mrb.h>
#include <caio.h>


#ifndef CHTTPD_RESPONSE_HEADER_BUFFSIZE
#define CHTTPD_RESPONSE_HEADER_BUFFSIZE 8192
#endif


#ifndef CHTTPD_REQUEST_HEADER_BUFFSIZE
#define CHTTPD_REQUEST_HEADER_BUFFSIZE 8192
#endif


#ifndef CHTTPD_REQUEST_STARTLINE_MAXLEN
#define CHTTPD_REQUEST_STARTLINE_MAXLEN CHTTPD_REQUEST_HEADER_BUFFSIZE
#endif


#ifndef CHTTPD_REQUEST_HEADERS_MAXCOUNT
#define CHTTPD_REQUEST_HEADERS_MAXCOUNT 64
#endif


#ifndef CHTTPD_URLARGS_MAXCOUNT
#define CHTTPD_URLARGS_MAXCOUNT 8
#endif


#ifndef CHTTPD_RESPONSE_BODY_MAXLEN
#define CHTTPD_RESPONSE_BODY_MAXLEN 8192
#endif


enum http_connection_token {
    HTTP_CT_NONE,
    HTTP_CT_KEEPALIVE,
    HTTP_CT_CLOSE,
};


struct chttpd_connection {
    /* state */
    bool closing;
    struct chttpd *chttpd;

    /* Connection */
    int fd;
    struct sockaddr remoteaddr;
    mrb_t inbuff;
    mrb_t outbuff;

    /* Startline buffer */
    char *startline;
    size_t startline_len;

    /* Header buffer */
    char *header;
    size_t header_len;

    /* HTTP headers */
    const char *headers[CHTTPD_REQUEST_HEADERS_MAXCOUNT];
    unsigned char headerscount;

    /* Attributes */
    const char *verb;
    const char *path;
    const char *version;
    enum http_connection_token connection;
    const char *contenttype;
    int contentlength;

    /* URL arguments */
    char *_url;
    const char *urlargs[CHTTPD_URLARGS_MAXCOUNT];
    unsigned int urlargscount;

    /* Handler */
    caio_coro handler;
};


/* Router entry */
struct chttpd_route {
    const char *pattern;
    const char *verb;
    caio_coro handler;
    regex_t preg;
};


/* chttpd state */
struct chttpd {
    /* Socket */
    const char *bindaddr;
    unsigned short bindport;
    struct sockaddr_storage listenaddr;
    socklen_t listenaddrlen;
    int listenfd;

    /* Limits */
    int backlog;
    size_t buffsize;
    size_t maxconn;

    /* Routes */
    struct chttpd_route *routes;
    caio_coro defaulthandler;
};


ASYNC
chttpdA(struct caio_task *self, struct chttpd *state);


int
chttpd_forever(struct chttpd *restrict state);


int
chttpd_response_flush(struct chttpd_connection *req);


ssize_t
chttpd_response_print(struct chttpd_connection *req, const char *format, ...);


ssize_t
chttpd_response(struct chttpd_connection *req, const char *restrict status,
        const char *restrict contenttype, const char *restrict format, ...);


/* Helper Macros */
#define CHTTPD_ROUTE(p, v, h) {(p), (v), (caio_coro)h}
#define CHTTPD_RESPONSE_FLUSH(req) while (chttpd_response_flush(req)) { \
        if (CORO_MUSTWAITFD()) { \
            CORO_WAITFD((req)->fd, CAIO_ET | CAIO_OUT); \
            continue; \
        } \
        req->closing = true; \
        CORO_RETURN; \
    }


/* Helper aliases, using macro for spped up. */
#define chttpd_response_write(r, d, c) mrb_putall((r)->outbuff, d, c)
#define chttpd_connection_close(r) (r)->closing = true


#endif  // CHTTPD_H_
