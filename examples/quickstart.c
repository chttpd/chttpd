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
_index(struct chttp_request *req, void *ptr) {
    // httpd_response_start(req, 200, NULL);
    // httpd_response_contenttype_set(req, "text/plain", "utf-8");
    // httpd_response_write(req, "Hello %s", __FILE__);
    // httpd_response_send(req);
    // return 0;
    return -1;
}


int
main() {
    chttpd_t server;
    clog_verbositylevel = CLOG_DEBUG;

    server = chttpd_new(&chttpd_defaultconfig);
    chttpd_route(server, "GET", "/", _index, NULL);
    return chttpd_main(server);
}
