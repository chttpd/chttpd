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

/* thirdparty */
#include <clog.h>

/* local public */
#include "chttpd/chttpd.h"


static int
_indexA(struct chttpd_connection *c, void *ptr) {
    if (14 != chttpd_responseA(c, 200, NULL, "Hello chttpd\r\n", 128)) {
        return -1;
    }

    return 0;
}


int
main() {
    chttpd_t server;
    clog_verbositylevel = CLOG_DEBUG;

    server = chttpd_new(&chttpd_defaultconfig);
    chttpd_route(server, "GET", "/", _indexA, NULL);
    return chttpd_main(server);
}
