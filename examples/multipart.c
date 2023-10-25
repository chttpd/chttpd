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


#define RESP_HEADER "<!doctype html><html><head>" \
    "<title>chttpd demo</title>" \
    "</head><body>"
#define RESP_FOOTER "</body></html>"


static ASYNC
indexA(struct caio_task *self, struct chttpd_connection *req) {
    struct chttpd_formfield *field = NULL;
    int flags = 0;
    CORO_START;

    do {
        CHTTPD_FORMFIELD_NEXT(req, &field, flags);

        // if (field
        switch (field->type) {
            case CHTTPD_FORMFIELD_TYPE_SCALAR:
            case CHTTPD_FORMFIELD_TYPE_FILE:
            case CHTTPD_FORMFIELD_TYPE_LIST:
            case CHTTPD_FORMFIELD_TYPE_OBJECT:
            default:
            // Invalid type
        }
    } while (field);

    chttpd_response(req, "200 OK", "text/html", RESP_HEADER
            "<h1>Hello %s!</h1>" RESP_FOOTER, "chttpd");
    CORO_FINALLY;
}


int
main() {
    struct chttpd state;

    clog_verbosity = CLOG_DEBUG;
    chttpd_defaults(&state);
    state.defaulthandler = (caio_coro)indexA;
    return chttpd(&state);
}
