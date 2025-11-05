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
    struct chttp_packet p;
    const char *buff;
    ssize_t bytes;

    ERR(chttp_packet_allocate(&p, 1, 16, CHTTP_TE_NONE));
    ERR(chttp_packet_startresponse(&p, 200, NULL));
    ERR(chttp_packet_contenttype(&p, "text/plain", "utf-8"));
    ERR(chttp_packet_transferencoding(&p, CHTTP_TE_CHUNKED));
    ERR(chttp_packet_close(&p));

    for (;;) {
        bytes = carrot_server_recvchunkA(c, &buff);
        if (bytes == -2) {
            /* buffer size is too low */
            break;
        }

        if (bytes == 0) {
            /* termination chunk just received */
            break;
        }

        if (bytes == -1) {
            /* malformed chunk */
            break;
        }

        ERR(chttp_packet_write(&p, buff, bytes));
        ASSRT(0 < carrot_server_sendpacketA(c, &p));
    }

    /* terminate */
    ASSRT(0 < carrot_server_sendpacketA(c, &p));
    return 0;
}


static void
test_request_chunked() {
    struct chttp_response *r = serverfixture_setup(1);
    isnotnull(r);
    route("GET", "/", _indexA, NULL);

    eqint(200, request("GET / HTTP/1.1\r\n"
                "Transfer-Encoding: chunked\r\n\r\n"
                "3\r\nfoo\r\n4\r\nquux\r\n0\r\n\r\n"));
    eqstr("Ok", r->text);
    eqint(CHTTP_TE_CHUNKED, r->transferencoding);
    eqstr("fooquux", content);

    serverfixture_teardown();
}


int
main() {
    test_request_chunked();
    return EXIT_SUCCESS;
}
