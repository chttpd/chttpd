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
_request_parse(struct chttpd_request *req, const char *header,
        int headerlen) {
    char *saveptr;
    char *linesaveptr;
    char *line;
    char *token;

    req->header = strndup(header, headerlen);
    if (req->header == NULL) {
        return -1;
    }
    /* Preserve header and it's len */
    req->headerlen = headerlen;

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
    DEBUG("t: %s", token);
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
    }

    return 0;

failed:
    free(req->header);
    return -1;
}


void
requestA(struct caio_task *self, struct chttpd_connection *conn) {
    CORO_START;

    if (conn->request) {
        // TODO: run handler
    }
    else {
        ssize_t headerlen = mrb_search(conn->inbuff, "\r\n\r\n", 4, 0, 8192);
        if (headerlen == -1) {
            conn->status = CCS_HEADER;
            CORO_RETURN;
        }
    }

    // TODO: Read the whole header
    // TODO: Allocate memory for request
    // TODO: Parse the request

    CORO_FINALLY;
}
