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

#include <clog.h>
#include <caio.h>

#include "chttpd.h"
#include "request.h"


int
chttpd_request_parse(struct chttpd_request *req) {
}


void
requestA(struct caio_task *self, struct chttpd_connection *conn) {
    CORO_START;

    // TODO: Read the whole header
    // TODO: Allocate memory for request
    // TODO: Parse the request

    CORO_FINALLY;
}
