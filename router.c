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


/**
 * https://developer-old.gnome.org/glib/stable/glib-regex-syntax.html
 */
int
chttpd_router_compilepatterns(struct chttpd_route * restrict route) {
    struct chttpd_route *r = route;

    while (r && r->pattern) {
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

    while (r && r->pattern) {
        regfree(&r->preg);
        r++;
    }
}


int
chttpd_route(struct chttpd_connection *req) {
    int g;
    int i = 0;
    struct chttpd *chttpd = req->chttpd;
    struct chttpd_route *r = chttpd->routes;
    regmatch_t pmatch[CHTTPD_URLARGS_MAXCOUNT + 1];

    while (r && r->pattern) {
        if (regexec(&r->preg, req->path, CHTTPD_URLARGS_MAXCOUNT + 1, pmatch,
                    0)
                == 0) {
            goto found;
        }
        r++;
        i++;
    }

    return -1;

found:
    req->_url = strdup(req->path);
    if (req->_url == NULL) {
        ERROR("Out of memory");
        return -1;
    }

    for (g = 1; g <= CHTTPD_URLARGS_MAXCOUNT; g++) {
        if (pmatch[g].rm_so == -1) {
            break;
        }

        req->urlargs[req->urlargscount++] = req->_url + pmatch[g].rm_so;
        req->_url[pmatch[g].rm_eo] = 0;
    }

    req->handler = r->handler;
    return i;
}
