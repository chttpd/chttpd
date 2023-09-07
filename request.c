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


int
chttpd_request_parse(struct chttpd_request *req, const char *header,
        int headerlen) {
    char *saveptr;
    char *token;
    char *buff = strndup(header, headerlen);
    if (buff == NULL) {
        return -1;
    }
    req->header = buff;

    /* Preserve headerlen */
    req->headerlen = headerlen;

    /* Verb */
    token = strtok_r(buff, " ", &saveptr);
    if (token == NULL) {
        goto failed;
    }

    /* Initialize the request fields */
    req->verb = token;
    req->contentlength = -1;

    /* Path */
    token = strtok_r(NULL, " ", &saveptr);
    if (token == NULL) {
        goto failed;
    }
    req->path = token;

    /* HTTP version */
    token = strtok_r(NULL, "/", &saveptr);
    if (token == NULL) {
        goto failed;
    }
    token = strtok_r(NULL, "\r\n", &saveptr);
    if (token == NULL) {
        goto failed;
    }
    req->version = token;

    /* Read headers */
    while ((token = strtok_r(NULL, "\r\n", &saveptr))) {
        if (strcasestr(token, "connection:") == token) {
            req->connection = trim(token + 11);
        }
        else if (strcasestr(token, "content-type:") == token) {
            req->contenttype = trim(token + 13);
        }
        else if (strcasestr(token, "content-length:") == token) {
            req->contentlength = atoi(trim(token + 15));
        }
    }

    return 0;

failed:
    free(buff);
    return -1;
}


void
requestA(struct caio_task *self, struct chttpd_connection *conn) {
    CORO_START;

    // TODO: Read the whole header
    // TODO: Allocate memory for request
    // TODO: Parse the request

    CORO_FINALLY;
}
