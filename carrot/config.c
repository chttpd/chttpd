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
/* standard */
#include <string.h>

/* local public */
#include "carrot/server.h"

/* local private */
#include "config.h"


const struct carrot_server_config carrot_server_defaultconfig = {
    .bind = "127.0.0.1:8080",
    .backlog = 10,
    .requestbuffer_mempages = 1,
    .connectionbuffer_mempages = 1,
    .connections_max = 10,
};


void
carrot_server_makedefaults(struct carrot_server_config *c) {
    memcpy(c, &carrot_server_defaultconfig,
            sizeof(carrot_server_defaultconfig));
}
