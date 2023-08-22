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

#include <clog.h>
#include <carrow.h>

#include <chttpd.h>


#define PAGESIZE 4096
#define BUFFSIZE (PAGESIZE * 32768)


static void
indexA(struct chttpd_request_coro *self, struct chttpd_request* req) {
    CORO_START;

    chttpd_response_start(req, "200 OK");
    chttpd_response_header(req, "Content-Type: %s; charset=%s", "text/plain",
            "us-ascii");

    /* Send response header */
    CHTTPD_RESPONSE_FLUSH(req);

    chttpd_response_body(req, "Foo, #%d\n", 1);
    chttpd_response_body(req, "Bar, #%d\n", 2);
    chttpd_response_body(req, "Baz, #%d\n", 3);

    chttpd_response_body(req, "{\"foo\": \"%s\"}", "Bar");
    CHTTPD_RESPONSE_FINALIZE(req);

    CORO_FINALLY;
}


static struct chttpd_route routes[] = {
    {"/", NULL, indexA},
    {NULL, NULL, NULL}
};


int
main() {
    clog_verbosity = CLOG_DEBUG;

    if (carrow_handleinterrupts()) {
        return EXIT_FAILURE;
    }

    struct chttpd state = {
        .bindaddr = "0.0.0.0",
        .bindport = 8080,
        .backlog = 2,
        .buffsize = BUFFSIZE,
        .routes = routes,
    };

    return chttpd_forever(chttpdA, &state, NULL);
}
