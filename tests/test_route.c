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

/* local private */
#include "router.h"
#include "common.h"

/* test private */
#include "tests/fixtures.h"


static void
test_route_find() {
    struct route *r;
    struct router router = {
        .count = 0,
    };

    eqint(0, router_append(&router, "GET", "/foo", NULL, NULL));

    r = router_find(&router, "GET", "/foo");
    isnotnull(r);
    eqstr("GET", r->verb);
    eqstr("/foo", r->path);

    isnull(router_find(&router, "GET", "/bar"));
}


int
main() {
    test_route_find();
    return EXIT_SUCCESS;
}
