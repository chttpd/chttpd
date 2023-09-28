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
#include "chttpd.h"
#include "router.h"


int
chttpd_router_compilepatterns(struct chttpd_route * restrict route) {
    struct chttpd_route *r = route;

    while (r->pattern) {
        if (regcomp(&r->preg, r->pattern, REG_EXTENDED)) {
            return -1;
        }
        r++;
    }

    return 0;
}


void
chttpd_router_cleanup(struct chttpd_route * restrict route) {
    struct chttpd_route *r = route;

    while (r->pattern) {
        regfree(&r->preg);
        r++;
    }
}


int
chttpd_route(struct chttpd_connection *req) {
    return -1;
}
