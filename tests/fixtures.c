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
#include "carrot/server.h"

/* local private */
#include "common.h"
#include "server.h"

/* test private */
#include "fixtures.h"


#define BUFFSIZE 4096
char content[BUFFSIZE];
static char _buff[BUFFSIZE];
static struct chttp_response *_resp  = NULL;
static struct carrot_server _carrot = {
    .config = &carrot_server_defaultconfig,
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

        s = chttp_chunked_parse(buff, len, &chunk, &garbage);
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
_clientA(int fd, ssize_t reqlen) {
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

    memset(&_carrot, 0, sizeof(_carrot));
    _carrot.listenfd = -1;
    _carrot.config = &carrot_server_defaultconfig;
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
    int client_status;
    int server_status;

    /* reset */
    errno = 0;
    chttp_response_reset(_resp);

    /* render request */
    va_start(args, fmt);
    bytes = vsnprintf(_buff, BUFFSIZE, fmt, args);
    va_end(args);

    /* ensure the output is not truncated */
    ERR(bytes >= BUFFSIZE);
    ERR(pcaio_modepoll_use(2, &modepoll));
    ERR(pcaio_modio_use(modepoll));
    ERR(_sockpair(socks, &caddr));

    // DEBUG("caddr: %s", saddr2a(&caddr));
    tasks[0] = pcaio_task_new(_clientA, &client_status, 2, socks[0], bytes);
    ASSRT(tasks[0]);

    tasks[1] = pcaio_task_new(server_connA, &server_status, 3, &_carrot,
            socks[1], &caddr);
    ASSRT(tasks[1]);

    /* run event loop */
    ERR(pcaio(1, tasks, 2));

    /* cleanup */
    close(socks[0]);
    close(socks[1]);
    ERR(ret);

    return client_status;
}


int
route(const char *verb, const char *path, carrot_handler_t handler,
        void *ptr) {
    return carrot_server_route(&_carrot, verb, path, handler, ptr);
}
