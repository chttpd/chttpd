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
#ifndef TESTS_FIXTURES_H_
#define TESTS_FIXTURES_H_


/* local public */
#include "chttpd/chttpd.h"


#define OK(e) if (e) return -1


chttp_status_t
request(const char *fmt, ...);


struct chttp_response *
serverfixture_setup(unsigned char pages);


void
serverfixture_teardown();


int
route(const char *verb, const char *path, chttpd_handler_t handler,
        void *ptr);


#endif  // TESTS_FIXTURES_H_
