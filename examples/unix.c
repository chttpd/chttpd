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

#include <chttpd.h>


static ASYNC
indexA(struct caio_task *self, struct chttpd_connection *req) {
    CORO_START;
    chttpd_response(req, "200 OK", "text/plain", NULL);
    CORO_FINALLY;
}


int
main() {
    struct chttpd state;

    clog_verbosity = CLOG_DEBUG;
    chttpd_defaults(&state);
    state.bindaddr = "unix:///tmp/chttpd_examples_unix.s",
    state.defaulthandler = (caio_coro)indexA;

    return chttpd(&state);
}
