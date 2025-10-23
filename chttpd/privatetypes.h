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
#ifndef CHTTPD_PRIVATETYPES_H_
#define CHTTPD_PRIVATETYPES_H_

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


struct router {
    struct route routes[CONFIG_CHTTPD_ROUTES_MAX];
    unsigned char count;
};


struct chttpd {
    const struct chttpd_config *config;
    int listenfd;
    struct router router;
};


#endif  // CHTTPD_PRIVATETYPES_H_
