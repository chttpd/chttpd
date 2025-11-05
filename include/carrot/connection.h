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
#ifndef INCLUDE_CARROT_CONNECTION_H_
#define INCLUDE_CARROT_CONNECTION_H_


/* thirdparty */
#include <mrb.h>
#include <chttp/chttp.h>

/* local public */
#include "carrot/addr.h"


struct carrot_connection {
    int fd;
    union saddr peer;
    struct mrb ring;
    union {
        struct chttp_request *request;
        struct chttp_response *response;
    };
};


ssize_t
carrot_connection_recvchunkA(struct carrot_connection *c, const char **start);


ssize_t
carrot_connection_recvsearchA(struct carrot_connection *c, const char *s);


int
carrot_connection_recvallA(struct carrot_connection *c, char **out);


ssize_t
carrot_connection_sendpacketA(struct carrot_connection *c,
        struct chttp_packet *p);


#endif  // INCLUDE_CARROT_CONNECTION_H_
