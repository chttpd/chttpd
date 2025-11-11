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

/* local public */
#include "carrot/server.h"


#define ERR(c) if (c) return -1
#define ASSRT(c) if (!(c)) return -1


static int
_chatA(struct carrot_connection *c, void *ptr) {
    const char *buff;
    ssize_t bytes;
    struct chttp_packet p;
    size_t total = 0;

    ERR(chttp_packet_allocate(&p, 1, 16, CHTTP_TE_NONE));
    ERR(chttp_packet_startresponse(&p, 200, NULL));
    ERR(chttp_packet_contenttype(&p, "text/plain", "utf-8"));
    ERR(chttp_packet_transferencoding(&p, CHTTP_TE_CHUNKED));
    ERR(chttp_packet_close(&p));

    for (;;) {
        bytes = carrot_connection_recvchunkA(c, &buff);
        if (bytes == -2) {
            ERROR("connection buffer size is too low");
            break;
        }

        if (bytes == -1) {
            ERROR("malformed chunk");
            break;
        }

        if (bytes == 0) {
            INFO("termination chunk just received");
            break;
        }

        total += bytes;
        INFO("echo chunksize: %ld", bytes);
        ERR(chttp_packet_write(&p, buff, bytes));
        ASSRT(0 < carrot_connection_sendpacketA(c, &p));
    }

    /* terminate */
    INFO("total: %ld", total);
    ASSRT(0 < carrot_connection_sendpacketA(c, &p));
    return 0;
}


static int
_streamA(struct carrot_connection *c, void *ptr) {
    struct chttp_packet p;

    ERR(chttp_packet_allocate(&p, 1, 1, CHTTP_TE_NONE));
    ERR(chttp_packet_startresponse(&p, 200, NULL));
    ERR(chttp_packet_contenttype(&p, "text/plain", "utf-8"));
    ERR(chttp_packet_transferencoding(&p, CHTTP_TE_CHUNKED));
    ERR(chttp_packet_close(&p));

    /* first chunk */
    ERR(chttp_packet_writef(&p, "Foo %s", "Bar"));
    ASSRT(0 < carrot_connection_sendpacketA(c, &p));

    /* second chunk */
    ERR(chttp_packet_writef(&p, " "));
    ERR(chttp_packet_writef(&p, "Baz %s", "Qux"));
    ERR(chttp_packet_writef(&p, "\r\n"));
    ASSRT(0 < carrot_connection_sendpacketA(c, &p));

    /* terminate */
    ASSRT(0 < carrot_connection_sendpacketA(c, &p));
    return 0;
}


static int
_indexA(struct carrot_connection *c, void *ptr) {
    int bytes = carrot_server_responseA(c, 200, NULL, "Hello carrot", -1,
            CARROT_SRF_APPENDCRLF);
    DEBUG("bytes: %d", bytes);
    return 0;
}


int
main() {
    carrot_server_t server;
    clog_verbositylevel = CLOG_DEBUG;
    struct carrot_server_config config;

    /* fill config variable with the default values, and override it */
    carrot_server_makedefaults(&config);
    config.connectionbuffer_mempages = 16;

    /* create a server */
    server = carrot_server_new(&config);

    /* add some routes */
    carrot_server_route(server, "POST", "/chat", _chatA, NULL);
    carrot_server_route(server, "GET", "/stream", _streamA, NULL);
    carrot_server_route(server, "GET", "/", _indexA, NULL);

    /* handover the process to server's entrypoint */
    return carrot_server_main(server);
}
