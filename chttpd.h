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


/**
 * @brief Asynchronous function that runs the chttpd server.
 *
 * It compiles the route patterns, listens for incoming connections, and
 * handles each connection asynchronously.
 *
 * @param self Pointer to the caio_task struct representing the coroutine.
 * @param chttpd Pointer to the chttpd struct representing the server.
 * @return ASYNC
 */
ASYNC
chttpdA(struct caio_task *self, struct chttpd *state);


/**
 * @brief Starts the chttpd server and runs it indefinitely.
 *
 * It performs some validation checks on the request and response buffer sizes
 * to ensure they are multiples of the system's page size. If the buffer sizes
 * are not valid, an error is logged, and the function returns -1. Otherwise,
 * it calls the CAIO to start the chttpd server with the specified maximum
 * number of connections.
 *
 * @param chttpd Pointer to the chttpd struct representing the server.
 * @return 0 on success, or -1 if an error occurs.
 */
int
chttpd_forever(struct chttpd *restrict state);


/**
 * @brief Sets the default values for a chttpd instance.
 *
 * It initializes various fields in the chttpd struct with default values.
 * These include the socket binding address and port, limits such as backlog
 * and maximum connections, buffer sizes for requests and responses, route
 * information, and hooks for connection and request events. The default
 * values are provided as constants or NULL pointers.
 *
 * @param chttpd Pointer to the chttpd struct to be initialized.
 */
void
chttpd_defaults(struct chttpd *restrict chttpd);


/* Request function, defined in request.c */
/**
 * @brief Retrieves the value of a specific header from the request.
 *
 * This function retrieves the value of a specific header from the given
 * chttpd_connection struct. It searches for the header with the specified
 * name in the array of headers stored in the chttpd_connection struct.
 * If a matching header is found, the function returns a pointer to the value
 * of the header. If no matching header is found, the function returns NULL.
 *
 * @param req Pointer to the chttpd_connection struct representing the
 *        connection and request.
 * @param name The name of the header to retrieve.
 * @return A pointer to the value of the header if found, NULL otherwise.
 */
const char *
chttpd_request_header_get(struct chttpd_connection *req, const char *name);


/* Response function, defined in response.c */
/**
 * @brief Flushes the response data to the client socket.
 *
 * This function flushes the response data stored in the output buffer
 * of the given chttpd_connection struct to the client socket. It writes
 * as much data as possible from the output buffer to the socket until the
 * output buffer becomes empty.
 *
 * @param req Pointer to the chttpd_connection struct representing the
 *        connection and response.
 * @return 0 on success, -1 on error.
 */
int
chttpd_response_flush(struct chttpd_connection *req);


/**
 * @brief Prints formatted data to the response output buffer.
 *
 * This function formats and prints data to the output buffer of the given
 * chttpd_connection struct representing the response. It accepts a format
 * string and variable number of arguments, similar to the printf function.
 * The formatted data is written to the output buffer using mrb_vprint.
 *
 * @param req Pointer to the chttpd_connection struct representing the
 *        connection and response.
 * @param format Format string specifying the format of the output.
 * @param ... Variable number of arguments to be formatted and printed.
 * @return The number of bytes written to the output buffer on success,
 *         or -1 on error.
 */
ssize_t
chttpd_response_print(struct chttpd_connection *req, const char *format, ...);


/**
 * @brief Generates an HTTP response with the given status, content type, and
 *        content.
 *
 * This function generates an HTTP response using the given chttpd_connection
 * structure, status, content type, and format string. It accepts a variable
 * number of parameters to be formatted and included in the response content.
 * The response is constructed by calling various chttpd_response_XXXX
 * functions and writing the data to the output buffer of the
 * chttpd_connection structure.
 *
 * @param req Pointer to the chttpd_connection structure representing the
 *        connection and response.
 * @param status The HTTP status line to be included in the response.
 * @param contenttype The MIME type of the response content.
 * @param format Format string specifying the format of the response content.
 * @param ... Variable number of arguments to be formatted and included in the
 *        response content.
 * @return The number of bytes written to the output buffer on success,
 *         or -1 on error.
 */
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
/**
 * @brief Parses string representation of an address and port into sockaddr
 * struct.
 *
 * This function takes a string representation of an address and port and
 * converts it into a sockaddr struct.
 *
 * @param saddr Pointer to the sockaddr struct to store the parsed address.
 * @param addr Pointer to the string representation of the address.
 * @param port The port number.
 * @return The size of the sockaddr struct on success, or -1 if on error.
 */
int
sockaddr_parse(struct sockaddr *saddr, const char *addr, unsigned short port);


/**
 * @brief Converts a sockaddr struct to a string representation.
 *
 * This function takes a sockaddr struct and converts it to a string
 * representation. It supports both AF_UNIX sockets and AF_INET sockets.
 *
 * @param addr Pointer to the sockaddr struct to be converted.
 * @return A pointer to the string representation of the sockaddr struct.
 */
char *
sockaddr_dump(struct sockaddr *addr);


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


/* Helper aliases, using macro for speed-up. */
#define chttpd_response_write(r, d, c) mrb_putall((r)->outbuff, d, c)
#define chttpd_connection_close(r) (r)->closing = true


#endif  // CHTTPD_H_
