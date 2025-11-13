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
#ifndef INCLUDE_CARROT_CLIENT_H_
#define INCLUDE_CARROT_CLIENT_H_


/* thirdparty */
#include <mrb.h>
#include <chttp/chttp.h>

/* local public */
#include "carrot/addr.h"
#include "carrot/connection.h"


typedef struct carrot_client *carrot_client_t;
struct carrot_client_config {
    unsigned int responsebuffer_mempages;
    unsigned int connectionbuffer_mempages;
};


void
carrot_client_makedefaults(struct carrot_client_config *c);


struct carrot_client *
carrot_client_new(const struct carrot_client_config *c);


void
carrot_client_free(struct carrot_client *c);


#endif  // INCLUDE_CARROT_CLIENT_H_
