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
    int ret;
    struct carrot_connection c;
    struct chttp_packet p;
    struct chttp_response *r;
    char *content;

    INFO("connecting to: %s", hostaddr);
    ERR(carrot_connectA(&c, cfg, hostaddr));
    // carrot_client_requestA(&c, "GET", "/");

    /* build and send a request */
    ERR(chttp_packet_allocate(&p, 1, 1, CHTTP_TE_NONE));
    ERR(chttp_packet_startrequest(&p, "GET", "/"));
    ERR(chttp_packet_contenttype(&p, "text/plain", "utf-8"));
    ERR(chttp_packet_writef(&p, "Hello carrot\r\n"));
    ERR(chttp_packet_close(&p));
    ret = carrot_connection_sendpacketA(&c, &p);
    chttp_packet_free(&p);
    ASSRT(ret > 0);

    ERR(carrot_client_waitresponseA(&c));
    r = c.response;
    printf("%d %s\r\n", r->status, r->text);

    if (r->contentlength > mrb_used(&c.ring)) {
        if (carrot_connection_recvallA(&c, &content) <= 0) {
            goto failed;
        }
    }
    printf("%.*s\r\n", (int)r->contentlength, mrb_readerptr(&c.ring));
    // /* make everything fresh for the next request */
    // mrb_reset(&c.ring);
    // chttp_response_reset(c.request);

failed:
    ERR(carrot_disconect(&c));
    return 0;
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
    client = pcaio_task_new(_clientA, NULL, 2, &c, "localhost:8080");
    ASSRT(client);

    /* execute and wait for pcaio event loop */
    return pcaio(1, &client, 1);
}
