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
#include <stddef.h>

/* thirdparty */
#include <clog.h>
#include <pcaio/pcaio.h>

/* local public */
#include "carrot/client.h"


#define ERR(c) if (c) return -1
#define ASSRT(c) if (!(c)) return -1


static int
_clinetA(struct carrot_clinet_config *cfg) {
    struct carrot_connection c;
    union saddr host;

    carrot_connect(&c, );
    // carrot_client_requestA(&c, "GET", "http://localhost:8080");
}


int
main() {
    pcaio_task_t client;
    struct pcaio_iomodule *modepoll;
    struct carrot_client_config c;

    /* clog verbosity */
    clog_verbositylevel = CLOG_DEBUG;

    /* carrot HTTP clinet configuration */
    carrot_client_makedefaults(&c);
    c.connectionbuffer_mempages = 16;

    /* rollup IO modules */
    ERR(pcaio_modepoll_use(8, &modepoll));
    ERR(pcaio_modio_use(modepoll));

    /* create a task for the clinet */
    clinet = pcaio_task_new(_clientA, NULL, 1, &c);
    ASSRT(client);

    /* execute and wait for pcaio event loop */
    return pcaio(1, &client, 1);
}
