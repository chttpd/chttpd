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
#include <clog.h>

/* local public */
#include "carrot/server.h"

/* local private */
#include "common.h"
#include "router.h"


struct route *
router_find(struct router *rt, const char *verb, const char *path) {
    int i;
    struct route *route = NULL;

    for (i = 0; i < rt->count; i++) {
        route = rt->routes + i;
        if ((strcmp(route->path, path) == 0)
                && (strcmp(route->verb, verb) == 0)) {
                return route;
        }
    }

    return NULL;
}


int
router_append(struct router *rt, const char *verb, const char *path,
        carrot_handler_t handler, void *ptr) {
    struct route *r;

    if (rt->count >= CONFIG_CARROT_ROUTES_MAX) {
        return -1;
    }

    r = &rt->routes[rt->count++];
    r->verb = verb;
    r->path = path;
    r->handler = handler;
    r->ptr = ptr;
    return 0;
}
