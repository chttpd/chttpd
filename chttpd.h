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


#ifndef CHTTPD_REQUEST_DEFAULT_BUFFSIZE
#define CHTTPD_REQUEST_DEFAULT_BUFFSIZE(pagesize) ((pagesize) * 8)
#endif


#ifndef CHTTPD_RESPONSE_DEFAULT_BUFFSIZE
#define CHTTPD_RESPONSE_DEFAULT_BUFFSIZE(pagesize) ((pagesize) * 8)
#endif


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
    const char *query;
    const char *version;
    const char *useragent;
    const char *contenttype;
    const char *accept;
    const char *expect;
    int contentlength;
    enum http_connection_token connection;

    /* URL arguments */
    char *_url;
    const char *urlargs[CHTTPD_URLARGS_MAXCOUNT];
    unsigned int urlargscount;

    /* Handler */
    caio_coro handler;
};


typedef int (*chttpd_connection_hook) (struct chttpd *chttpd, int fd,
        struct sockaddr addr);
typedef int (*chttpd_request_hook) (struct chttpd_connection *req);


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
    size_t maxconn;
    size_t request_buffsize;
    size_t response_buffsize;

    /* Routes */
    struct chttpd_route *routes;
    caio_coro defaulthandler;

    /* Hooks */
    chttpd_connection_hook on_connection_open;
    chttpd_connection_hook on_connection_close;
    chttpd_request_hook on_request_begin;
    chttpd_request_hook on_request_end;
};


ASYNC
chttpdA(struct caio_task *self, struct chttpd *state);


int
chttpd_forever(struct chttpd *restrict state);


void
chttpd_defaults(struct chttpd *restrict chttpd);


/* Request function, defined in request.c */
const char *
chttpd_request_header_get(struct chttpd_connection *req, const char *name);


/* Response function, defined in response.c */
int
chttpd_response_flush(struct chttpd_connection *req);


ssize_t
chttpd_response_print(struct chttpd_connection *req, const char *format, ...);


ssize_t
chttpd_response(struct chttpd_connection *req, const char *restrict status,
        const char *restrict contenttype, const char *restrict format, ...);


/* Query string functions, defined in querystring.c */
int
chttpd_querystring_tokenize(char *query, char **saveptr, char **key,
        char **value);


/* Helper functions, defined in helpers.c */
char *
trim(char *s);


int
urldecode(char *encoded);


/* Networking helpers */
int
sockaddr_parse(struct sockaddr *saddr, const char *addr, unsigned short port);


char *
sockaddr_dump(struct sockaddr *addr);


/* Helper Macros */
#define CHTTPD_ROUTE(p, v, h) {(p), (v), (caio_coro)h}
#define CHTTPD_ROUTE_TERMINATOR CHTTPD_ROUTE(NULL, NULL, NULL)
#define CHTTPD_RESPONSE_FLUSH(req) while (chttpd_response_flush(req)) { \
        if (CORO_MUSTWAITFD()) { \
            CORO_WAITFD((req)->fd, CAIO_ET | CAIO_OUT); \
            continue; \
        } \
        req->closing = true; \
        CORO_RETURN; \
    }


/* Helper aliases, using macro for speed-up. */
#define chttpd_response_write(r, d, c) mrb_putall((r)->outbuff, d, c)
#define chttpd_connection_close(r) (r)->closing = true


#endif  // CHTTPD_H_
