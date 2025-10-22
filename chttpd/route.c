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
#include <clog.h>

/* local public */
#include "chttpd/chttpd.h"

/* local private */
#include "privatetypes.h"
#include "route.h"


struct route *
route_find(struct chttpd *s, struct chttp_request *r) {
    int i;
    struct route *route = NULL;

    for (i = 0; i < s->routescount; i++) {
        route = s->routes + i;
        if ((strncmp(route->path, r->path, strlen(route->path)) == 0)
                && (strcmp(route->verb, r->verb) == 0)) {
                return route;
        }
    }

    return NULL;
}


int
chttpd_route(struct chttpd *s, const char *verb, const char *path,
        chttpd_handler_t handler, void *ptr) {
    struct route *r;

    if (s->routescount >= CONFIG_CHTTPD_ROUTES_MAX) {
        return -1;
    }

    r = &s->routes[s->routescount++];
    r->verb = verb;
    r->path = path;
    r->handler = handler;
    r->ptr = ptr;
    return 0;
}
