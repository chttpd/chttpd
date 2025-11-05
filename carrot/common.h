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
#ifndef CARROT_COMMON_H_
#define CARROT_COMMON_H_

/* local private */
#include "config.h"

/* local public */
#include "carrot/carrot.h"


/* private preprocessors */
#define ERR(c) if (c) return -1
#define ASSRT(c) if (!(c)) return -1


struct route {
    const char *verb;
    const char *path;
    carrot_handler_t handler;
    void *ptr;
};


struct router {
    struct route routes[CONFIG_CARROT_ROUTES_MAX];
    unsigned char count;
};


struct carrot {
    const struct carrot_config *config;
    int listenfd;
    struct router router;
};


#endif  // CARROT_COMMON_H_
