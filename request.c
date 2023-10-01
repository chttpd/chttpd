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
#include "chttpd.h"
#include "request.h"


void
chttpd_request_reset(struct chttpd_connection *req) {
    if (req == NULL) {
        return;
    }

    /* Startline buffer */
    req->startline_len = 0;
    if (req->startline != NULL) {
        free(req->startline);
        req->startline = NULL;
    }

    /* Header buffer */
    req->header_len = 0;
    if (req->header != NULL) {
        free(req->header);
        req->header = NULL;
    }

    /* HTTP headers */
    req->headerscount = 0;

    /* Attributes */
    req->verb = NULL;
    req->path = NULL;
    req->query = NULL;
    req->version = NULL;
    req->connection = HTTP_CT_NONE;
    req->contenttype = NULL;
    req->accept = NULL;
    req->useragent = NULL;
    req->contentlength = -1;
    req->urlargscount = 0;

    if (req->_url) {
        free(req->_url);
        req->_url = NULL;
    }

    /* Handler */
    req->handler = NULL;
}


const char *
chttpd_request_header_get(struct chttpd_connection *req, const char *name) {
    int i;
    const char *header;

    for (i = 0; i < req->headerscount; i++) {
        header = req->headers[i];
        if (strcasestr(header, name) == header) {
            return trim((char *)(header + strlen(name) + 1));
        }
    }

    return NULL;
}


int
chttpd_request_startline_parse(struct chttpd_connection *req) {
    char *saveptr;
    char *token;
    char *tmp;
    ssize_t bytes;
    ssize_t sllen;

    /* Try to parse request start line */
    while (true) {
search:
        if (!mrb_isempty(req->inbuff)) {
            /* Try to read the start line */
            sllen = mrb_search(req->inbuff, "\r\n", 2, 0,
                    CHTTPD_REQUEST_STARTLINE_MAXLEN);
            if (sllen == 0) {
                /* Ignore starting dummy newline */
                mrb_skip(req->inbuff, 2);
                goto search;
            }
            if (sllen > 0) {
                break;
            }

            if (mrb_used(req->inbuff) >= CHTTPD_REQUEST_STARTLINE_MAXLEN) {
                errno = 0;
                goto failed;
            }
        }

        errno = 0;
        bytes = mrb_readin(req->inbuff, req->fd, mrb_available(req->inbuff));
        if (bytes <= 0) {
            goto failed;
        }
    }

    /* Allocate memory for startline */
    req->startline = malloc(sllen + 1);
    if (req->startline == NULL) {
        ERROR("Out of memory");
        goto failed;
    }
    req->startline_len = sllen;

    /* Read the start line from the request buffer */
    mrb_get(req->inbuff, req->startline, sllen);
    req->startline[sllen] = 0;

    /* Verb */
    token = strtok_r(req->startline, " ", &saveptr);
    if (token == NULL) {
        goto failed;
    }

    /* Initialize the request fields */
    req->verb = token;

    /* Path */
    token = strtok_r(NULL, " ", &saveptr);
    if (token == NULL) {
        goto failed;
    }
    req->path = token;

    /* Query string */
    tmp = strchr(token, '?');
    if (tmp) {
        tmp[0] = '\0';
        req->query = ++tmp;
    }
    urldecode(token);

    /* HTTP version */
    token = strtok_r(NULL, " ", &saveptr);
    if (token) {
        if (strncmp("HTTP/", token, 5)) {
            goto failed;
        }
        req->version = token + 5;
    }

    return 0;

failed:
    return -1;
}


int
chttpd_request_headers_parse(struct chttpd_connection *req) {
    char *saveptr;
    char *line;
    char *tmp;
    ssize_t bytes;
    ssize_t headerlen;

    /* Try gather request header */
    while (true) {
        /* Check the whole header is available or not */
        headerlen = mrb_search(req->inbuff, "\r\n\r\n", 4, 0,
                CHTTPD_REQUEST_HEADER_BUFFSIZE);
        if (headerlen == 0) {
            return 0;
        }
        else if (headerlen > 0) {
            break;
        }

        // TODO: Preserve searched area to improve performance.
        if (mrb_isfull(req->inbuff) ||
                (mrb_used(req->inbuff) >= CHTTPD_REQUEST_HEADER_BUFFSIZE)) {
            errno = 0;
            goto failed;
        }

        errno = 0;
        bytes = mrb_readin(req->inbuff, req->fd, mrb_available(req->inbuff));
        if (bytes <= 0) {
            goto failed;
        }
    }

    /* Skip the primitive(startline) CRLF */
    headerlen -= 2;
    mrb_skip(req->inbuff, 2);

    /* Allocate memory for request header */
    req->header = malloc(headerlen + 1);
    if (req->header == NULL) {
        goto failed;
    }
    req->header_len = headerlen;

    /* Read whole HTTP header from the request buffer */
    mrb_get(req->inbuff, req->header, headerlen);
    req->header[headerlen] = 0;

    /* Skip the second CRLF */
    mrb_skip(req->inbuff, 2);

    /* Parse the request */
    req->headerscount = 0;

    /* Read headers */
    line = strtok_r(req->header, "\r\n", &saveptr);
    do {
        if (strcasestr(line, "connection:") == line) {
            tmp = trim(line + 11);
            if (strcasecmp("close", tmp) == 0) {
                req->connection = HTTP_CT_CLOSE;
            }
            else if (strcasecmp("keep-alive", tmp) == 0) {
                req->connection = HTTP_CT_KEEPALIVE;
            }
            else {
                errno = 0;
                goto failed;
            }
        }
        else if (strcasestr(line, "content-type:") == line) {
            req->contenttype = trim(line + 13);
        }
        else if (strcasestr(line, "accept:") == line) {
            req->accept = trim(line + 7);
        }
        else if (strcasestr(line, "expect:") == line) {
            req->expect = trim(line + 7);
            if (strcmp("100-continue", req->expect)) {
                errno = 0;
                goto failed;
            }
        }
        else if (strcasestr(line, "user-agent:") == line) {
            req->useragent = trim(line + 11);
        }
        else if (strcasestr(line, "content-length:") == line) {
            req->contentlength = atoi(trim(line + 15));
        }
        else if (req->headerscount < (CHTTPD_REQUEST_HEADERS_MAXCOUNT - 1)) {
            req->headers[req->headerscount++] = line;
        }
        else {
            errno = 0;
            goto failed;
        }
    } while ((line = strtok_r(NULL, "\r\n", &saveptr)));

    return 0;

failed:
    return -1;
}


int
chttpd_request_parse(struct chttpd_connection *req) {
    if ((req->startline == NULL) && chttpd_request_startline_parse(req)) {
        return -1;
    }

    if ((req->header == NULL) && chttpd_request_headers_parse(req)) {
        return -1;
    }

    return 0;
}
