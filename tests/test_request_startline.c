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


static void
test_request_startline() {
    struct chttp_response *r = chttp_response_new(1);

    eqint(-1, testreq(r, "foo"));
    eqint(400, testreq(r, "GET / HTTP/1.1"));
    // eqint(400, r->status);
    // eqstr("Bad Request", r->text);

    chttp_response_free(r);
}


int
main() {
    test_request_startline();
    return EXIT_SUCCESS;
}
