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
#ifndef TESTS_FIXTURES_H_
#define TESTS_FIXTURES_H_


/* local public */
#include "carrot/carrot.h"


/* private preprocessors */
#define ERR(c) if (c) return -1
#define ASSRT(c) if (!(c)) return -1


extern char content[];


chttp_status_t
request(const char *fmt, ...);


struct chttp_response *
serverfixture_setup(unsigned char pages);


void
serverfixture_teardown();


int
route(const char *verb, const char *path, carrot_handler_t handler,
        void *ptr);


#endif  // TESTS_FIXTURES_H_
