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
#include <stdlib.h>

#include <cutest.h>
#include <clog.h>

#include "chttpd.h"
#include "request.c"


void
test_request_parse() {
    char *request;
    struct chttpd_request req;

    request = strdup(
        "GET /foo/bar HTTP/1.1\r\n"
        "Connection: close\r\n"
        "Content-Type: qux/quux\r\n"
        "Host: foo\r\n"
        "Content-Length: 124\r\n"
        "Foo: bar\r\n"
        "\r\n");

    memset(&req, 0, sizeof(req));
    eqint(0, _request_parse(&req, request, strlen(request)));
    eqstr("GET", req.verb);
    eqstr("/foo/bar", req.path);
    eqstr("1.1", req.version);
    eqstr(req.connection, "close");
    eqstr(req.contenttype, "qux/quux");
    eqint(124, req.contentlength);
    eqstr("foo", chttpd_request_header_get(&req, "host"));
    eqstr("bar", chttpd_request_header_get(&req, "foo"));
    isnull(chttpd_request_header_get(&req, "bar"));
    free(req.header);

    /* Header with no value */
    request = strdup(
        "GET /foo/bar HTTP/1.1\r\n"
        "Connection:\r\n");

    memset(&req, 0, sizeof(req));
    eqint(0, _request_parse(&req, request, strlen(request)));
    eqstr("GET", req.verb);
    eqstr("/foo/bar", req.path);
    eqstr("1.1", req.version);
    eqstr(req.connection, "");
    isnull(req.contenttype);
    eqint(-1, req.contentlength);
    free(req.header);

    /* Missing headers */
    request = strdup("GET /foo/bar HTTP/1.1\r\n");

    memset(&req, 0, sizeof(req));
    eqint(0, _request_parse(&req, request, strlen(request)));
    eqstr("GET", req.verb);
    eqstr("/foo/bar", req.path);
    eqstr("1.1", req.version);
    isnull(req.connection);
    isnull(req.contenttype);
    eqint(-1, req.contentlength);
    free(req.header);

    /* Missing version */
    request = strdup("GET /\r\n");
    memset(&req, 0, sizeof(req));
    eqint(0, _request_parse(&req, request, strlen(request)));
    eqstr("GET", req.verb);
    eqstr("/", req.path);
    isnull(req.version);
    free(req.header);

    /* Bad version */
    request = strdup("GET / FOO\r\n");
    memset(&req, 0, sizeof(req));
    eqint(0, _request_parse(&req, request, strlen(request)));
    eqstr("GET", req.verb);
    eqstr("/", req.path);
    eqstr("FOO", req.version);
    free(req.header);
}


void
main() {
    test_request_parse();
}
