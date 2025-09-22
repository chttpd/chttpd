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
#ifndef INCLUDE_CHTTPD_CHTTPD_H_
#define INCLUDE_CHTTPD_CHTTPD_H_


/* thirdparty */
#include <chttp.h>


typedef int (*chttpd_handler_t)(struct chttp_request *req, void *ptr);
typedef struct chttpd *chttpd_t;
struct chttpd_config {
    const char *bind;
    unsigned short backlog;
};


void
chttpd_config_default(struct chttpd_config *c);


struct chttpd *
chttpd_new(struct chttpd_config *c);


void
chttpd_free(struct chttpd *s);


int
chttpd_route(struct chttpd *s, const char *verb, const char *path,
        chttpd_handler_t handler, void *ptr);


int
chttpd_main(struct chttpd *s);


int
chttpdA(int argc, void *argv[]);


#endif  // INCLUDE_CHTTPD_CHTTPD_H_
