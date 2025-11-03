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
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <socket.h>

/* thirdparty */
#include <clog.h>
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>
#include <pcaio/modepoll.h>

/* local public */
#include "chttpd/chttpd.h"

/* local private */
#include "privatetypes.h"
#include "connection.h"

/* test private */
#include "fixtures.h"


#define BUFFSIZE 4096
char content[BUFFSIZE];
static char _buff[BUFFSIZE];
static struct chttp_response *_resp  = NULL;
static struct chttpd _chttpd = {
    .config = &chttpd_defaultconfig,
    .listenfd = -1,
    .router = {
        .count = 0,
    }
};


static int
_chunkedA(const char *buff, int len, int avail, int fd) {
    ssize_t s;
    char *cursor = content;
    const char *chunk;
    size_t garbage;
    ssize_t ret;

    for (;;) {
        if (avail == 0) {
            return -1;
        }

        s = chttp_chunkedcodec_getchunk(buff, len, &chunk, &garbage);
        if (s == -1) {
            return -1;
        }

        if (s == -2) {
            pcaio_relaxA(0);
            ret = readA(fd, (void *)buff + len, avail);
            if (ret <= 0) {
                return -1;
            }

            len += ret;
            avail -= ret;
            continue;
        }

        if (s == 0) {
            cursor[0] = 0;
            break;
        }

        memcpy(cursor, chunk, s);
        cursor += s;
        buff += garbage;
    }

    return 0;
}


static int
_clientA(int argc, void *argv[]) {
    int fd = (long)argv[0];
    ssize_t reqlen = (long)argv[1];
    ssize_t bytes;
    chttp_status_t status;
    char *header;
    int headerlen;

    bytes = writeA(fd, _buff, reqlen);
    if (bytes < reqlen) {
        return -1;
    }

    bytes = readA(fd, _buff, BUFFSIZE);
    if (bytes <= 0) {
        ERROR("readA");
        return -1;
    }

    header = memmem(_buff, bytes, "\r\n\r\n", 4);
    if (header == NULL) {
        return -1;
    }

    header += 2;
    headerlen = header - _buff;
    if (chttp_response_parse(_resp, _buff, headerlen)) {
        ERROR("chttp_response_parse");
        return -1;
    }

    status = _resp->status;
    pcaio_relaxA(0);

    headerlen += 2;
    if ((_resp->transferencoding & CHTTP_TE_CHUNKED)
            && _chunkedA(_buff + headerlen, bytes - headerlen,
                BUFFSIZE - headerlen, fd)) {
        return -1;
    }

    close(fd);
    return status;
}


static int
_sockpair(int socks[2], union saddr *caddr) {
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, socks)) {
        return -1;
    }

    // TODO: mock socket address
    memset(caddr, 0, sizeof(union saddr));
    return 0;
}


struct chttp_response *
serverfixture_setup(unsigned char pages) {
    if (_resp) {
        return NULL;
    }

    _resp = chttp_response_new(pages);
    if (_resp == NULL) {
        return NULL;
    }

    return _resp;
}


void
serverfixture_teardown() {
    if (_resp) {
        chttp_response_free(_resp);
        _resp = NULL;
    }

    memset(&_chttpd, 0, sizeof(_chttpd));
    _chttpd.listenfd = -1;
    _chttpd.config = &chttpd_defaultconfig;
}


chttp_status_t
request(const char *fmt, ...) {
    chttp_status_t ret = 0;
    va_list args;
    int socks[2];
    int bytes;
    struct pcaio_task *tasks[2];
    struct pcaio_iomodule *modepoll;
    union saddr caddr;
    int client_exitstatus;
    int server_exitstatus;

    /* reset */
    errno = 0;
    chttp_response_reset(_resp);

    /* render request */
    va_start(args, fmt);
    bytes = vsnprintf(_buff, BUFFSIZE, fmt, args);
    va_end(args);

    /* ensure the output is not truncated */
    ASSRT(bytes >= BUFFSIZE);
    ASSRT(pcaio_modepoll_use(2, &modepoll));
    ASSRT(pcaio_modio_use(modepoll));
    ASSRT(_sockpair(socks, &caddr));

    DEBUG("caddr: %s", saddr2a(&caddr));
    tasks[0] = pcaio_task_new(_clientA, &client_exitstatus, 2, socks[0],
            bytes);
    ASSRT(!tasks[0]);

    tasks[1] = pcaio_task_new(connectionA, &server_exitstatus, 3, &_chttpd,
            socks[1], &caddr);
    ASSRT(!tasks[1]);

    /* run event loop */
    ASSRT(pcaio(1, tasks, 2));

    /* cleanup */
    close(socks[0]);
    close(socks[1]);
    ASSRT(ret);

    return client_exitstatus;
}


int
route(const char *verb, const char *path, chttpd_handler_t handler,
        void *ptr) {
    return chttpd_route(&_chttpd, verb, path, handler, ptr);
}
