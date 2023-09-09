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
#include <ctype.h>

#include <clog.h>
#include <mrb.h>
#include <caio.h>

#include "chttpd.h"
#include "request.h"


static char *
trim(char *s) {
    if (s == NULL) {
        return NULL;
    }
    int l = strlen(s);

    while (s[0] && isspace(s[0])) {
        s++;
        l--;
    }

    while (isspace(s[l -1])) {
        s[--l] = 0;
    }

    return s;
}


static int
_request_parse(struct chttpd_request *req, char *header, int headerlen) {
    char *saveptr;
    char *linesaveptr;
    char *line;
    char *token;

    /* Preserve header and it's len */
    req->header = header;
    req->headerlen = headerlen;
    req->headerscount = 0;

    /* Protocol's first line */
    line = strtok_r(req->header, "\r\n", &saveptr);
    if (line == NULL) {
        goto failed;
    }

    /* Verb */
    token = strtok_r(line, " ", &linesaveptr);
    if (token == NULL) {
        goto failed;
    }

    /* Initialize the request fields */
    req->verb = token;
    req->contentlength = -1;

    /* Path */
    token = strtok_r(NULL, " ", &linesaveptr);
    if (token == NULL) {
        goto failed;
    }
    req->path = token;

    /* HTTP version */
    token = strtok_r(NULL, "/", &linesaveptr);
    if (token) {
        req->version = token;
        token = strtok_r(NULL, "\r\n", &linesaveptr);
        if (token) {
            req->version = token;
        }
    }
    else {
        req->version = NULL;
    }

    /* Read headers */
    while ((line = strtok_r(NULL, "\r\n", &saveptr))) {
        if (strcasestr(line, "connection:") == line) {
            req->connection = trim(line + 11);
        }
        else if (strcasestr(line, "content-type:") == line) {
            req->contenttype = trim(line + 13);
        }
        else if (strcasestr(line, "content-length:") == line) {
            req->contentlength = atoi(trim(line + 15));
        }
        else if (req->headerscount < CHTTPD_REQUESTHEADERS_MAX) {
            req->headers[req->headerscount++] = line;
        }
        else {
            goto failed;
        }
    }

    return 0;

failed:
    free(req->header);
    return -1;
}


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


const char *
chttpd_request_header_get(struct chttpd_request *req, const char *name) {
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


void
requestA(struct caio_task *self, struct chttpd_request *req) {
    char *header;
    ssize_t headerlen;
    CORO_START;
    int hsize = CHTTPD_HEADERSIZE + 4;

    if (req->status == CCS_HEADER) {
        /* Check the whole header is available or not */
        headerlen = mrb_search(req->inbuff, "\r\n\r\n", 4, 0, hsize);
        if (headerlen == -1) {
            // TODO: Preserve searched area to improve performance.
            if (mrb_used(req->inbuff) >= hsize) {
                req->status = CCS_CLOSING;
            }
            else {
                req->status = CCS_HEADER;
            }
            CORO_RETURN;
        }
        headerlen += 2;

        /* Allocate memory for request header */
        header = malloc(headerlen + 1);
        if (header == NULL) {
            req->status = CCS_CLOSING;
            CORO_RETURN;
        }

        /* Read the HTTP header from request buffer */
        if (mrb_get(req->inbuff, header, headerlen) != headerlen) {
            req->status = CCS_CLOSING;
            CORO_RETURN;
        }

        /* Parse the request */
        if (_request_parse(req, header, headerlen)) {
            /* Request parse error */
            req->status = CCS_CLOSING;
            CORO_RETURN;
        }
    }

    // TODO: Find handler
    // TODO: Dispatch

    CORO_FINALLY;
    chttpd_request_free(req);
}
