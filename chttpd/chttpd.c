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
#include <stddef.h>

/* local private */
#include "config.h"

/* local public */
#include "chttpd/chttpd.h"


struct route {
    const char *verb;
    const char *path;
    chttpd_handler_t handler;
    void *ptr;
};


struct chttpd {
    int fd;
    int backlog;
    unsigned char routescount;
    struct route routes[CONFIG_CHTTPD_ROUTES_MAX];
};


void
chttpd_config_default(struct chttpd_config *c) {
    c->bind = "127.0.0.1:80";
    c->backlog = 10;
}


struct chttpd *
chttpd_new(struct chttpd_config *c) {
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


int
chttpdA(int argc, void *argv[]) {
    return -1;
}
