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
static char _buff[BUFFSIZE];
struct chttpd _chttpd = {
    .config = &chttpd_defaultconfig,
    .listenfd = -1,
    .router = {
        .count = 0,
    }
};


static int
_clientA(int argc, void *argv[]) {
    int fd = (long)argv[0];
    ssize_t reqlen = (long)argv[1];
    struct chttp_response *r = (struct chttp_response*)argv[2];
    ssize_t bytes;
    chttp_status_t status;
    char *content;

    bytes = writeA(fd, _buff, reqlen);
    if (bytes < reqlen) {
        return -1;
    }

    bytes = readA(fd, _buff, BUFFSIZE);
    if (bytes <= 0) {
        ERROR("readA");
        return -1;
    }

    content = memmem(_buff, bytes, "\r\n\r\n", 4);
    if (content == NULL) {
        return -1;
    }

    content += 2;
    if (chttp_response_parse(r, _buff, content - _buff)) {
        ERROR("chttp_response_parse");
        return -1;
    }

    status = r->status;
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


chttp_status_t
testreq(struct chttp_response *r, const char *fmt, ...) {
    chttp_status_t ret = 0;
    va_list args;
    int socks[2];
    int bytes;
    struct pcaio_task *tasks[2];
    struct pcaio_iomodule *modepoll;
    union saddr caddr;
    int client_exitstatus;
    int server_exitstatus;

    /* reset the response */
    chttp_response_reset(r);

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
    tasks[0] = pcaio_task_new(_clientA, &client_exitstatus, 3, socks[0],
            bytes, r);
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
