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
#ifndef CARROT_SERVER_H_
#define CARROT_SERVER_H_


/* local private */
#include "router.h"


struct carrot_server {
    const struct carrot_server_config *config;
    int listenfd;
    struct router router;
};


int
server_connA(struct carrot_server *s, int fd, union saddr *peer);


#endif  // CARROT_SERVER_H_
