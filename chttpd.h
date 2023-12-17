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
#include <caio/caio.h>


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


/* HTTP form */
enum chttpd_formfield_type {
    CHTTPD_FORMFIELD_TYPE_SCALAR,
    CHTTPD_FORMFIELD_TYPE_FILE,
    CHTTPD_FORMFIELD_TYPE_LIST,
    CHTTPD_FORMFIELD_TYPE_OBJECT,
};


enum chttpd_formtype {
    CHTTPD_FORMTYPE_UNKNOWN,
    CHTTPD_FORMTYPE_URLENCODED,
    CHTTPD_FORMTYPE_MULTIPART,
    CHTTPD_FORMTYPE_JSON,
};


struct chttpd_form;
struct chttpd_connection;
struct chttpd_formfield;
typedef struct chttpd_form chttpd_form_t;


#undef CAIO_ARG1
#undef CAIO_ARG2
#undef CAIO_ENTITY
#define CAIO_ENTITY chttpd_form
#define CAIO_ARG1 struct chttpd_formfield **
#define CAIO_ARG2 int
#include "caio/generic.h"


typedef struct chttpd_form {
    struct chttpd_connection *req;
    enum chttpd_formtype type;
    chttpd_form_coro nextfield;
} chttpd_form_t;


struct chttpd_formfield {
    enum chttpd_formfield_type type;
    const char *name;
    union {
        const char *value;
    } u;
};


enum http_connection_token {
    HTTP_CONNECTIONTOKEN_NONE,
    HTTP_CONNECTIONTOKEN_KEEPALIVE,
    HTTP_CONNECTIONTOKEN_CLOSE,
};


typedef struct chttpd_connection {
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

    /* State */
    int remainingbytes;

    /* Handler */
    caio_coro handler;

    /* Form */
    struct chttpd_form *form;
} chttpd_connection_t;


#undef CAIO_ARG1
#undef CAIO_ARG2
#undef CAIO_ENTITY
#define CAIO_ENTITY chttpd_connection
#include "caio/generic.h"  // NOLINT


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
typedef struct chttpd {
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
} chttpd_t;


#undef CAIO_ARG1
#undef CAIO_ARG2
#undef CAIO_ENTITY
#define CAIO_ENTITY chttpd
#include "caio/generic.h"  // NOLINT


ASYNC
chttpdA(struct caio_task *self, struct chttpd *state);


int
chttpd(struct chttpd *restrict state);


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


/* HTTP Form */
int
chttpd_form_new(struct chttpd_connection *req);


ASYNC
body_readA(struct caio_task *self, struct chttpd_connection *req);


ASYNC
chttpd_formfield_next(struct caio_task *self, struct chttpd_form *form,
        struct chttpd_formfield **out, int flags);


/* Helper aliases, using macro for speeds-up. */
#define CHTTPD_RESPONSE_WRITE(r, d, c) mrb_putall((r)->outbuff, d, c)
#define CHTTPD_CONNECTION_CLOSE(r) (r)->closing = true
#define CHTTPD_RESPONSE_TEXT(r, s, fmt, ...) \
    chttpd_response(r, s, "text/html", fmt , # __VA_ARGS__)


/* Router Macros */
#define CHTTPD_ROUTE(p, v, h) {(p), (v), (caio_coro)h}
#define CHTTPD_ROUTE_TERMINATOR CHTTPD_ROUTE(NULL, NULL, NULL)


/* Response Macros */
#define CHTTPD_RESPONSE_FLUSH(req) while (chttpd_response_flush(req)) { \
        if (CORO_MUSTWAITFD()) { \
            CORO_WAITFD((req)->fd, CAIO_ET | CAIO_OUT); \
            continue; \
        } \
        req->closing = true; \
        CORO_RETURN; \
    }


/* HTTP content types */
#define HTTP_CONTENTTYPE_FORM_URLENCODED "application/x-www-form-urlencoded"
#define HTTP_CONTENTTYPE_FORM_MULTIPART "multipart/form-data"
#define HTTP_CONTENTTYPE_FORM_JSON "application/json"

/* HTTP status codes */
/* 1xx */
#define HTTPSTATUS_CONTINUE             "100 Continue"
#define HTTPSTATUS_SWITCHINGPROTOCOLS   "101 Switching Protocols"
#define HTTPSTATUS_EARLYHINTS           "103 Early Hints"

/* 2xx */
#define HTTPSTATUS_OK                   "200 OK"
#define HTTPSTATUS_CREATED              "201 Created"
#define HTTPSTATUS_ACCEPTED             "202 Accepted"
#define HTTPSTATUS_NOCONTENT            "204 No Content"
#define HTTPSTATUS_RESETCONTENT         "205 Reset Content"
#define HTTPSTATUS_PARTIALCONTENT       "206 Partial Content"
#define HTTPSTATUS_IMUSED               "226 IM Used"

/* 3xx */
#define HTTPSTATUS_MULTIPLECHOICES      "300 Multiple Choices"
#define HTTPSTATUS_MOVEDPERMANENTLY     "301 Moved Permanently"
#define HTTPSTATUS_FOUND                "302 Moved temporarily"
#define HTTPSTATUS_SEEOTHER             "303 See Other"
#define HTTPSTATUS_NOTMODIFIED          "304 Not Modified"
#define HTTPSTATUS_USEPROXY             "305 Use Proxy"
#define HTTPSTATUS_SWITCHPROXY          "306 Switch Proxy"
#define HTTPSTATUS_TEMPORARYREDIRECT    "307 Temporary Redirect"
#define HTTPSTATUS_PERMANENTREDIRECT    "308 Permanent Redirect"

/* 4xx */
#define HTTPSTATUS_BADREQUEST           "400 Bad Request"
#define HTTPSTATUS_UNAUTHORIZED         "401 Unauthorized"
#define HTTPSTATUS_PAYMENTREQUIRED      "402 Payment Required"
#define HTTPSTATUS_FORBIDDEN            "403 Forbidden"
#define HTTPSTATUS_NOTFOUND             "404 Not Found"
#define HTTPSTATUS_METHODNOTALLOWED     "405 Method Not Allowed"
#define HTTPSTATUS_NOTACCEPTABLE        "406 Not Acceptable"
#define HTTPSTATUS_REQUESTTIMEOUT       "408 Request Timeout"
#define HTTPSTATUS_CONFLICT             "409 Conflict"
#define HTTPSTATUS_GONE                 "410 Gone"
#define HTTPSTATUS_LENGTHREQUIRED       "411 Length Required"
#define HTTPSTATUS_PRECONDITIONFAILED   "412 Precondition Failed"
#define HTTPSTATUS_PAYLOADTOOLARGE      "413 Payload Too Large"
#define HTTPSTATUS_URITOOLONG           "414 URI Too Long"
#define HTTPSTATUS_UNSUPPORTEDMEDIATYPE "415 Unsupported Media Type"
#define HTTPSTATUS_RANGENOTSATISFIABLE  "416 Range Not Satisfiable"
#define HTTPSTATUS_EXPECTATIONFAILED    "417 Expectation Failed"
#define HTTPSTATUS_MISDIRECTEDREQUEST   "421 Misdirected Request"
#define HTTPSTATUS_UNPROCESSABLEENTITY  "422 Unprocessable Entity"
#define HTTPSTATUS_UPGRADEREQUIRED      "426 Upgrade Required"
#define HTTPSTATUS_PRECONDITIONREQUIRED "428 Precondition Required"
#define HTTPSTATUS_TOOMANYREQUESTS      "429 Too Many Requests"

/* 5xx */
#define HTTPSTATUS_INTERNALSERVERERROR  "500 Internal Server Error"
#define HTTPSTATUS_NOTIMPLEMENTED       "501 Not Implemented"
#define HTTPSTATUS_BADGATEWAY           "502 Bad Gateway"
#define HTTPSTATUS_SERVICEUNAVAILABLE   "503 Service Unavailable"
#define HTTPSTATUS_GATEWAYTIMEOUT       "504 Gateway Timeout"


#define CHTTPD_STATUS_INTERNALSERVERERROR(r) \
    CHTTPD_RESPONSE_TEXT(r, HTTPSTATUS_INTERNALSERVERERROR, \
            "Internal Server Error"CR); \
    CHTTPD_CONNECTION_CLOSE(r)


/* Form Macros */
#define CHTTPD_FORMFIELD_NEXT(req, field, flags) \
    if (req->form == NULL) { \
        if (chttpd_form_new(req)) { \
            ERROR("Out of memory"); \
            CHTTPD_STATUS_INTERNALSERVERERROR(req); \
            CORO_RETURN; \
        } \
    } \
    if (req->form->nextfield) { \
        AWAIT(chttpd_form, (req)->form->nextfield, (req)->form, field, flags); \
    }


#define STARTSWITH(a, b) (strncasecmp(a, b, strlen(b)) == 0)


enum chttpd_eno {
    CHTTPD_ENO_OK,
    CHTTPD_ENO_CONTENTLENGTHMISSING,
    CHTTPD_ENO_REQUESTTOOLONG,
    CHTTPD_ENO_CONNECTIONCLOSED,
};


#endif  // CHTTPD_H_
