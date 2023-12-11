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
#include <cutest.h>
#include <clog.h>

#include "chttpd.h"
#include "helpers.c"
#include "connection.c"
#include "form.c"
#include "request.c"

#include "testhelpers.h"


void
test_request_form_urlencoded() {
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
        "Content-Length: 15\r\n\r\n"
        "foo=bar&baz=qux\r\n");
    eqint(0, chttpd_request_parse(&req));

    chttpd_request_reset(&req);
    mrb_destroy(req.inbuff);
    mrb_destroy(req.outbuff);
    fclose(infile.file);
}


int
main() {
    test_request_form_urlencoded();
    return EXIT_SUCCESS;
}
