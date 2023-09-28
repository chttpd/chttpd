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

#include <cutest.h>

#include "chttpd.h"
#include "router.c"


static ASYNC
indexA(struct caio_task *self, struct chttpd_connection *conn) {
}


static ASYNC
fooA(struct caio_task *self, struct chttpd_connection *conn) {
}


void
test_router() {
    struct chttpd_route routes[] = {
        CHTTPD_ROUTE("^/foo$", NULL, fooA),
        CHTTPD_ROUTE("^/foo/(.*)$", NULL, fooA),
        CHTTPD_ROUTE("^/$", NULL, indexA),
        CHTTPD_ROUTE(NULL, NULL, NULL)
    };
    struct chttpd chttpd = {
        .routes = routes,
    };
    struct chttpd_connection req = {
        .verb = "GET",
        .chttpd = &chttpd,
    };

    chttpd_router_compilepatterns(routes);

    req.path = "/bar";
    req.urlargscount = 0;
    eqint(-1, chttpd_route(&req));
    isnull(req.handler);
    eqint(0, req.urlargscount);

    req.path = "/foo";
    req.urlargscount = 0;
    eqint(0, chttpd_route(&req));
    eqptr(fooA, req.handler);
    eqint(0, req.urlargscount);

    req.path = "/";
    req.urlargscount = 0;
    eqint(2, chttpd_route(&req));
    eqptr(indexA, req.handler);
    eqint(0, req.urlargscount);

    req.path = "/foo/bar";
    req.urlargscount = 0;
    eqint(1, chttpd_route(&req));
    eqptr(fooA, req.handler);
    eqint(1, req.urlargscount);
    eqstr("bar", req.urlargs[0]);

    chttpd_router_cleanup(routes);
}


void
main() {
    test_router();
}
