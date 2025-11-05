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
#ifndef CARROT_ROUTER_H_
#define CARROT_ROUTER_H_


/* local private */
#include "common.h"


struct route {
    const char *verb;
    const char *path;
    carrot_handler_t handler;
    void *ptr;
};


struct router {
    struct route routes[CONFIG_CARROT_SERVER_MAXROUTES];
    unsigned char count;
};


struct route *
router_find(struct router *rt, const char *verb, const char *path);


int
router_append(struct router *rt, const char *verb, const char *path,
        carrot_handler_t handler, void *ptr);


#endif  // CARROT_ROUTER_H_
