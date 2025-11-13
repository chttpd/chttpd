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

/* posix */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/* thirdparty */
#include <clog.h>
#include <pcaio/pcaio.h>

/* local public */
#include "carrot/client.h"

/* local private */
#include "common.h"
#include "client.h"


const struct carrot_client_config carrot_client_defaultconfig = {
    .responsebuffer_mempages = 1,
    .connectionbuffer_mempages = 1,
};


void
carrot_client_makedefaults(struct carrot_client_config *c) {
    memcpy(c, &carrot_client_defaultconfig,
            sizeof(carrot_client_defaultconfig));
}


int
carrot_connectA(struct carrot_client_config *cfg, const char *saddr) {
    // struct addrinfo hints;
    // struct addrinfo *result;
    // const char *node;
    // const char *service;

    // ERR(saddr_split(saddr, &node, &service));
    // getaddrinfo(node, service, &hints, &result)
    return -1;
}
