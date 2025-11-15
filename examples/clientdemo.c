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
#include <stdio.h>

/* thirdparty */
#include <clog.h>
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>
#include <pcaio/modepoll.h>

/* local public */
#include "carrot/client.h"


#define ERR(c) if (c) return -1
#define ASSRT(c) if (!(c)) return -1


static int
_clientA(struct carrot_client_config *cfg, const char *hostaddr) {
    int ret = 0;
    struct carrot_connection c;
    struct chttp_packet p;
    struct chttp_response *r;

    ERR(carrot_client_connectA(&c, cfg, hostaddr));
    r = c.response;

    /* build and send a request */
    ERR(chttp_packet_allocate(&p, 1, 1, CHTTP_TE_NONE));
    chttp_packet_startrequest(&p, "GET", "/");
    chttp_packet_contenttype(&p, "text/plain", "utf-8");
    chttp_packet_writef(&p, "Hello carrot\r\n");
    chttp_packet_close(&p);

    ret = carrot_client_queryA(&c, &p);
    chttp_packet_free(&p);
    if (ret == 0) {
        printf("%d %s\r\n", r->status, r->text);
        printf("%.*s", (int)r->contentlength, mrb_readerptr(&c.ring));
    }

    ERR(carrot_client_disconnect(&c));
    return ret;
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
    client = pcaio_task_new(_clientA, NULL, 2, &c, "google.com:80");
    ASSRT(client);

    /* execute and wait for pcaio event loop */
    return pcaio(1, &client, 1);
}
