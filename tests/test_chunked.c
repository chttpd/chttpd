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


#define BS 128


static int
_indexA(struct chttpd_connection *c, void *ptr) {
    const char *buff;
    ssize_t bytes;

    OK(chttpd_response_start(c, 200, NULL));
    OK(chttpd_response_header(c, "Transfer-Encoding: chunked"));
    OK(chttpd_response_header_close(c));
    OK(0 >= chttpd_response_header_flushA(c));

    for (;;) {
        bytes = chttpd_request_readchunkA(c, &buff);
        if (bytes > BS) {
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

        OK(0 >= chttpd_response_writechunkA(c, buff, bytes));
    }

    /* terminate */
    OK(0 >= chttpd_response_chunk_end(c));
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
