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
test_request_parse_complex() {
    const char *in;
    struct chttpd_connection req;
    memset(&req, 0, sizeof(req));
    struct tfile infile = tmpfile_open();
    req.fd = infile.fd;
    istrue(req.fd > 2);
    req.inbuff = mrb_create(CHTTPD_REQUEST_HEADER_BUFFSIZE);
    req.outbuff = mrb_create(CHTTPD_REQUEST_HEADER_BUFFSIZE);
    chttpd_request_reset(&req);

    /* Complex request */
    REQ("GET /foo/bar?foo=bar&baz=qux HTTP/1.1\r\n"
        "Connection: close\r\n"
        "Content-Type: qux/quux\r\n"
        "Host: foo\r\n"
        "Content-Length: 124\r\n"
        "Accept: */*\r\n"
        "Expect: 100-continue\r\n"
        "User-Agent: foobar\r\n"
        "Foo: bar\r\n\r\n");
    eqint(0, chttpd_request_parse(&req));
    eqstr("GET", req.verb);
    eqstr("/foo/bar", req.path);
    eqstr("1.1", req.version);
    eqstr("foo=bar&baz=qux", req.query);
    eqint(HTTP_CT_CLOSE, req.connection);
    eqstr("qux/quux", req.contenttype);
    eqstr("foobar", req.useragent);
    eqstr("*/*", req.accept);
    eqstr("100-continue", req.expect);
    eqint(124, req.contentlength);
    eqstr("foo", chttpd_request_header_get(&req, "host"));
    eqstr("bar", chttpd_request_header_get(&req, "foo"));
    isnull(chttpd_request_header_get(&req, "content-type"));
    isnull(chttpd_request_header_get(&req, "content-length"));
    isnull(chttpd_request_header_get(&req, "bar"));
    eqint(138, req.header_len);
    eqint(37, req.startline_len);
    chttpd_request_reset(&req);

    /* Primitive CRLF */
    REQ("\r\nGET /foo/bar HTTP/1.1\r\n"
        "Connection: close\r\n"
        "Content-Type: qux/quux\r\n"
        "Host: foo\r\n"
        "Content-Length: 124\r\n"
        "Foo: bar\r\n\r\n");
    eqint(0, chttpd_request_parse(&req));
    eqstr("GET", req.verb);
    eqstr("/foo/bar", req.path);
    eqstr("1.1", req.version);
    eqint(HTTP_CT_CLOSE, req.connection);
    eqstr("qux/quux", req.contenttype);
    isnull(req.useragent);
    isnull(req.accept);
    eqint(124, req.contentlength);
    eqstr("foo", chttpd_request_header_get(&req, "host"));
    eqstr("bar", chttpd_request_header_get(&req, "foo"));
    isnull(chttpd_request_header_get(&req, "content-type"));
    isnull(chttpd_request_header_get(&req, "content-length"));
    isnull(chttpd_request_header_get(&req, "bar"));
    eqint(83, req.header_len);

    chttpd_request_reset(&req);
    mrb_destroy(req.inbuff);
    mrb_destroy(req.outbuff);
    fclose(infile.file);
}


void
test_request_parse_headers() {
    const char *in;
    struct chttpd_connection req;
    memset(&req, 0, sizeof(req));
    struct tfile infile = tmpfile_open();
    req.fd = infile.fd;
    istrue(req.fd > 2);
    req.inbuff = mrb_create(CHTTPD_REQUEST_HEADER_BUFFSIZE);
    req.outbuff = mrb_create(CHTTPD_REQUEST_HEADER_BUFFSIZE);
    chttpd_request_reset(&req);

    REQ("GET / HTTP/1.1\r\n"
        "Connection: close\r\n"
        "Content-Type: qux/quux\r\n"
        "Host: foo\r\n"
        "Content-Length: 124\r\n"
        "Foo: bar\r\n"
        "Bar: baz\r\n\r\n");
    eqint(0, chttpd_request_parse(&req));
    eqint(HTTP_CT_CLOSE, req.connection);
    eqstr("qux/quux", req.contenttype);
    eqint(124, req.contentlength);
    eqstr("foo", chttpd_request_header_get(&req, "host"));
    eqstr("bar", chttpd_request_header_get(&req, "foo"));
    eqstr("baz", chttpd_request_header_get(&req, "bar"));
    isnull(chttpd_request_header_get(&req, "content-type"));
    isnull(chttpd_request_header_get(&req, "content-length"));
    isnull(chttpd_request_header_get(&req, "baz"));
    chttpd_request_reset(&req);

    /* Header with no value */
    REQ("GET /foo/bar HTTP/1.1\r\n"
        "Foo:\r\n\r\n");
    eqint(0, chttpd_request_parse(&req));
    eqstr("GET", req.verb);
    eqstr("/foo/bar", req.path);
    eqstr("1.1", req.version);
    isnull(req.contenttype);
    eqint(HTTP_CT_NONE, req.connection);
    eqint(-1, req.contentlength);
    chttpd_request_reset(&req);

    /* Missing headers */
    REQ("GET /foo/bar HTTP/1.1\r\n\r\n");
    eqint(0, chttpd_request_parse(&req));
    eqstr("GET", req.verb);
    eqstr("/foo/bar", req.path);
    eqstr("1.1", req.version);
    eqint(HTTP_CT_NONE, req.connection);
    isnull(req.contenttype);
    eqint(-1, req.contentlength);

    chttpd_request_reset(&req);
    mrb_destroy(req.inbuff);
    mrb_destroy(req.outbuff);
    fclose(infile.file);
}


void
test_request_parse_malformed() {
    const char *in;
    struct chttpd_connection req;
    memset(&req, 0, sizeof(req));
    struct tfile infile = tmpfile_open();
    req.fd = infile.fd;
    istrue(req.fd > 2);
    req.inbuff = mrb_create(CHTTPD_REQUEST_HEADER_BUFFSIZE);
    req.outbuff = mrb_create(CHTTPD_REQUEST_HEADER_BUFFSIZE);
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
    eqint(-1, chttpd_request_parse(&req));
    chttpd_request_reset(&req);

    /* Malformed request */
    REQ("foo");
    eqint(-1, chttpd_request_parse(&req));
    eqint(0, errno);
    chttpd_request_reset(&req);

    REQ("foo\r\n");
    eqint(-1, chttpd_request_parse(&req));
    eqint(0, errno);
    chttpd_request_reset(&req);

    /* Empty line */
    REQ("\r\n");
    eqint(-1, chttpd_request_parse(&req));
    eqint(0, errno);
    chttpd_request_reset(&req);

    /* Bad line */
    REQ("\n");
    eqint(-1, chttpd_request_parse(&req));
    eqint(0, errno);
    chttpd_request_reset(&req);

    /* Bad line */
    REQ("\r");
    eqint(-1, chttpd_request_parse(&req));
    eqint(0, errno);
    chttpd_request_reset(&req);

    /* Empty request */
    REQ("");
    eqint(-1, chttpd_request_parse(&req));
    eqint(0, errno);
    chttpd_request_reset(&req);

    /* Connection header with no value */
    REQ("GET /foo/bar HTTP/1.1\r\n"
        "Connection:\r\n\r\n");
    eqint(-1, chttpd_request_parse(&req));
    chttpd_request_reset(&req);

    /* Invalid expect header */
    REQ("GET / HTTP/1.1\r\n"
        "Expect: foo\r\n\r\n");
    eqint(-1, chttpd_request_parse(&req));
    chttpd_request_reset(&req);

    mrb_destroy(req.inbuff);
    mrb_destroy(req.outbuff);
    fclose(infile.file);
}


void
test_request_parse_urlencoded() {
    const char *in;
    struct chttpd_connection req;
    memset(&req, 0, sizeof(req));
    struct tfile infile = tmpfile_open();
    req.fd = infile.fd;
    istrue(req.fd > 2);
    req.inbuff = mrb_create(CHTTPD_REQUEST_HEADER_BUFFSIZE);
    req.outbuff = mrb_create(CHTTPD_REQUEST_HEADER_BUFFSIZE);
    chttpd_request_reset(&req);

    /* Url encoded */
    REQ("GET /foo%20bar HTTP/1.1\r\n\r\n");
    eqint(0, chttpd_request_parse(&req));
    eqstr("/foo bar", req.path);
    chttpd_request_reset(&req);

    REQ("GET /foo%20bar?baz=qux%20thud HTTP/1.1\r\n\r\n");
    eqint(0, chttpd_request_parse(&req));
    eqstr("/foo bar", req.path);
    eqstr("baz=qux%20thud", req.query);
    chttpd_request_reset(&req);

    mrb_destroy(req.inbuff);
    mrb_destroy(req.outbuff);
    fclose(infile.file);
}


int
main() {
    test_request_parse_complex();
    test_request_parse_headers();
    test_request_parse_malformed();
    test_request_parse_urlencoded();
    return EXIT_SUCCESS;
}
