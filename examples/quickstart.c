// Copyright 2025 Vahid Mardani
/*
 * This file is part of chttpd.
 *  chttpd is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  chttpd is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with chttpd. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Author: Vahid Mardani <vahid.mardani@gmail.com>
 */
/* standard */
#include <stddef.h>

/* thirdparty */
#include <clog.h>

/* local public */
#include "chttpd/chttpd.h"


#define ERR(c) if (c) return -1
#define ASSRT(c) if (!(c)) return -1


static int
_chatA(struct chttpd_connection *c, void *ptr) {
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
        bytes = chttpd_request_readchunkA(c, &buff);
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
        ASSRT(0 < chttpd_connection_sendpacket(c, &p));
    }

    /* terminate */
    INFO("total: %ld", total);
    ASSRT(0 < chttpd_connection_sendpacket(c, &p));
    return 0;
}


static int
_streamA(struct chttpd_connection *c, void *ptr) {
    struct chttp_packet p;

    ERR(chttp_packet_allocate(&p, 1, 1, CHTTP_TE_NONE));
    ERR(chttp_packet_startresponse(&p, 200, NULL));
    ERR(chttp_packet_contenttype(&p, "text/plain", "utf-8"));
    ERR(chttp_packet_transferencoding(&p, CHTTP_TE_CHUNKED));
    ERR(chttp_packet_close(&p));

    /* first chunk */
    ERR(chttp_packet_writef(&p, "Foo %s", "Bar"));
    ASSRT(0 < chttpd_connection_sendpacket(c, &p));

    /* second chunk */
    ERR(chttp_packet_writef(&p, " "));
    ERR(chttp_packet_writef(&p, "Baz %s", "Qux"));
    ERR(chttp_packet_writef(&p, "\r\n"));
    ASSRT(0 < chttpd_connection_sendpacket(c, &p));

    /* terminate */
    ASSRT(0 < chttpd_connection_sendpacket(c, &p));
    return 0;
}


static int
_indexA(struct chttpd_connection *c, void *ptr) {
    int bytes = chttpd_responseA(c, 200, NULL, "Hello chttpd\r\n", 128);
    DEBUG("bytes: %d", bytes);
    return 0;
}


int
main() {
    chttpd_t server;
    clog_verbositylevel = CLOG_DEBUG;
    struct chttpd_config config;

    chttpd_config_makedefaults(&config);
    config.connectionbuffer_mempages = 16;
    server = chttpd_new(&config);
    chttpd_route(server, "POST", "/chat", _chatA, NULL);
    chttpd_route(server, "GET", "/stream", _streamA, NULL);
    chttpd_route(server, "GET", "/", _indexA, NULL);
    return chttpd_main(server);
}
