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


#define OK(e) if (e) return -1
#define BS 0x10000


static int
_chatA(struct chttpd_connection *c, void *ptr) {
    char *buff;
    ssize_t bytes;

    OK(chttpd_response_start(c, 200, NULL));
    OK(chttpd_response_contenttype(c, "text/plain", "utf-8"));
    OK(chttpd_response_header(c, "Transfer-Encoding: chunked"));
    OK(chttpd_response_header_close(c));
    OK(0 >= chttpd_response_header_flushA(c));

    buff = malloc(BS);
    if (buff == NULL) {
        ERROR("insufficient memory");
    }

    for (;;) {
        bytes = chttpd_request_readchunkA(c, buff, BS);
        if (bytes > BS) {
            ERROR("buffer size is too low: %d", bytes);
            break;
        }

        if (bytes == 0) {
            INFO("termination chunk just received");
            break;
        }

        if (bytes == -1) {
            ERROR("malformed chunk");
            break;
        }

        INFO("echo chunksize: %ld", bytes);
        OK(0 >= chttpd_response_writechunkA(c, buff, bytes));
    }

    /* terminate */
    free(buff);
    OK(0 >= chttpd_response_chunk_end(c));
    return 0;
}


static int
_streamA(struct chttpd_connection *c, void *ptr) {
    OK(chttpd_response_start(c, 200, NULL));
    OK(chttpd_response_contenttype(c, "text/plain", "utf-8"));
    OK(chttpd_response_header(c, "Transfer-Encoding: chunked"));
    OK(chttpd_response_header_close(c));
    OK(0 >= chttpd_response_header_flushA(c));

    /* allocate chunk size */
    OK(chttpd_response_allocate(c, 128));

    /* first chunk */
    OK(0 >= chttpd_response_write(c, "Foo %s", "Bar"));
    OK(chttpd_response_flushchunkA(c));

    /* second chunk */
    OK(0 >= chttpd_response_write(c, " "));
    OK(0 >= chttpd_response_write(c, "Baz %s", "Qux"));
    OK(0 >= chttpd_response_write(c, "\r\n"));
    OK(chttpd_response_flushchunkA(c));

    /* terminate */
    OK(0 >= chttpd_response_chunk_end(c));
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
    // config.connectionbuffer_mempages = 16;
    server = chttpd_new(&config);
    chttpd_route(server, "POST", "/chat", _chatA, NULL);
    chttpd_route(server, "GET", "/stream", _streamA, NULL);
    chttpd_route(server, "GET", "/", _indexA, NULL);
    return chttpd_main(server);
}
