// Copyright 2025 Vahid Mardani
/*
 * This file is part of carrot.
 *  carrot is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  carrot is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with carrot. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Author: Vahid Mardani <vahid.mardani@gmail.com>
 */
/* standard */
/* thirdparty */
#include <cutest.h>

/* local public */
#include "carrot/server.h"

/* test private */
#include "tests/fixtures.h"


static int
_indexA(struct carrot_server_conn *c, void *ptr) {
    int i;
    struct chttp_request *r = c->request;
    struct chttp_packet p;

    ERR(chttp_packet_allocate(&p, 1, 1, CHTTP_TE_NONE));
    ERR(chttp_packet_startresponse(&p, 200, NULL));
    ERR(chttp_packet_headerf(&p, "x-headers: %d", r->headers.count));

    /* copy request headers to response headers */
    for (i = 0; i < r->headers.count; i++) {
        ERR(chttp_packet_headerf(&p, r->headers.list[i]));
    }

    ERR(chttp_packet_close(&p));
    ASSRT(0 < carrot_server_sendpacketA(c, &p));
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
    serverfixture_teardown();
}


int
main() {
    test_request_headers();
    test_request_startline();
    return EXIT_SUCCESS;
}
