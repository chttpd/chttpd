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
#include <unistd.h>
#include <sys/mman.h>

#include <cutest.h>
#include <clog.h>

#include "chttpd.h"
#include "helpers.c"
#include "request.c"


struct tfile {
    FILE *file;
    int fd;
};


static struct tfile
tmpfile_open() {
    struct tfile t = {
        .file = tmpfile(),
    };

    t.fd = fileno(t.file);
    return t;
}


#define REQ(r) \
    in = (r); \
    lseek(req.fd, 0, SEEK_SET); \
    ftruncate(req.fd, 0); \
    write(req.fd, in, strlen(in)); \
    lseek(req.fd, 0, SEEK_SET)


void
test_request_parse() {
    const char *in;
    struct chttpd_connection req;
    memset(&req, 0, sizeof(req));
    struct tfile infile = tmpfile_open();
    req.fd = infile.fd;
    istrue(req.fd > 2);
    req.inbuff = mrb_create(CHTTPD_REQUEST_HEADER_BUFFSIZE);
    req.outbuff = mrb_create(CHTTPD_REQUEST_HEADER_BUFFSIZE);
    chttpd_request_reset(&req);

    REQ("GET /foo/bar HTTP/1.1\r\n"
        "Connection: close\r\n"
        "Content-Type: qux/quux\r\n"
        "Host: foo\r\n"
        "Content-Length: 124\r\n"
        "Foo: bar\r\n\r\n");
    eqint(0, chttpd_request_parse(&req));
    eqstr("GET", req.verb);
    eqstr("/foo/bar", req.path);
    eqstr("1.1", req.version);
    eqstr(req.connection, "close");
    eqstr(req.contenttype, "qux/quux");
    eqint(124, req.contentlength);
    eqstr("foo", chttpd_request_header_get(&req, "host"));
    eqstr("bar", chttpd_request_header_get(&req, "foo"));
    isnull(chttpd_request_header_get(&req, "content-type"));
    isnull(chttpd_request_header_get(&req, "content-length"));
    isnull(chttpd_request_header_get(&req, "bar"));
    eqint(83, req.header_len);
    chttpd_request_reset(&req);

    /* Header with no value */
    REQ("GET /foo/bar HTTP/1.1\r\n"
        "Connection:\r\n\r\n");
    eqint(0, chttpd_request_parse(&req));
    eqstr("GET", req.verb);
    eqstr("/foo/bar", req.path);
    eqstr("1.1", req.version);
    eqstr(req.connection, "");
    isnull(req.contenttype);
    eqint(-1, req.contentlength);
    chttpd_request_reset(&req);

    /* Missing headers */
    REQ("GET /foo/bar HTTP/1.1\r\n\r\n");
    eqint(0, chttpd_request_parse(&req));
    eqstr("GET", req.verb);
    eqstr("/foo/bar", req.path);
    eqstr("1.1", req.version);
    isnull(req.connection);
    isnull(req.contenttype);
    eqint(-1, req.contentlength);
    chttpd_request_reset(&req);

    /* Missing version */
    REQ("GET /\r\n\r\n");
    eqint(0, chttpd_request_parse(&req));
    eqstr("GET", req.verb);
    eqstr("/", req.path);
    isnull(req.version);
    chttpd_request_reset(&req);

    /* Bad version */
    REQ("GET / FOO\r\n\r\n");
    eqint(0, chttpd_request_parse(&req));
    eqstr("GET", req.verb);
    eqstr("/", req.path);
    eqstr("FOO", req.version);
    chttpd_request_reset(&req);

    mrb_destroy(req.inbuff);
    mrb_destroy(req.outbuff);
    fclose(infile.file);
}


int
main() {
    test_request_parse();
    return EXIT_SUCCESS;
}
