// Copyright 2025 Vahid Mardani
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
/* standard */
/* thirdparty */
#include <cutest.h>

/* local public */
#include "chttpd/chttpd.h"

/* test private */
#include "tests/fixtures.h"


static int
_indexA(struct chttpd_connection *c, void *ptr) {
    int i;
    struct chttp_request *r = c->request;

    OK(-1 == chttpd_response_start(c, 200, NULL));
    OK(chttpd_response_header(c, "x-headers: %d", r->headers.count));

    /* copy request headers to response headers */
    for (i = 0; i < r->headers.count; i++) {
        OK(chttpd_response_header(c, r->headers.list[i]));
    }

    OK(chttpd_response_header_close(c));
    OK(-1 == chttpd_response_flushA(c));
    return 0;
}


static void
test_request_headers() {
    struct chttp_response *r = serverfixture_setup(1);
    isnotnull(r);
    route("GET", "/", _indexA, NULL);

    eqint(200, request("GET / HTTP/1.1\r\n"
                "x-foo: bar\r\n\r\n"));
    eqint(2, r->headers.count);
    eqstr("1", chttp_headerset_get(&r->headers, "x-headers"));
    eqstr("bar", chttp_headerset_get(&r->headers, "x-foo"));

    eqint(200, request("GET / HTTP/1.1\r\n\r\n"));
    eqint(1, r->headers.count);
    eqstr("0", chttp_headerset_get(&r->headers, "x-headers"));

    serverfixture_teardown();
}


static void
test_request_startline() {
    struct chttp_response *r = serverfixture_setup(1);
    isnotnull(r);

    eqint(400, request("foo\r\n\r\n"));
    eqint(400, request("GET HTTP/1.1\r\n\r\n"));
    eqint(400, r->status);
    eqstr("Bad Request", r->text);

    eqint(404, request("GET / HTTP/1.1\r\n\r\n"));
    route("GET", "/", _indexA, NULL);
    serverfixture_teardown();
}


int
main() {
    test_request_headers();
    test_request_startline();
    return EXIT_SUCCESS;
}
