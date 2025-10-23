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


struct chttpd_connection testconn;
struct chttpd _chttpd = {
    .config = &chttpd_defaultconfig,
    .fd = -1,
    .routescount = 0,
};


static int
_clientA(int argc, void *argv[]) {
    int fd = (long)argv[0];
    char *req = (char *)argv[1];
    ssize_t reqlen = (long)argv[2];
    ssize_t bytes;

    bytes = writeA(fd, req, reqlen);
    if (bytes < reqlen) {
        return -1;
    }

    return 0;
}


static int
_sockpair(int socks[2], union saddr *caddr) {
    socklen_t addrlen;

    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, socks)) {
        return -1;
    }

    addrlen = sizeof(struct sockaddr_storage);
    if (getsockname(socks[0], &caddr->sa, &addrlen)) {
        close(socks[0]);
        close(socks[1]);
        return -1;
    }

    return 0;
}


chttp_status_t
testreq(const char *fmt, ...) {
    chttp_status_t ret = 0;
    va_list args;
    char buff[4096];
    int socks[2];
    int bytes;
    struct pcaio_task *tasks[2];
    struct pcaio_iomodule *modepoll;
    union saddr caddr;

    va_start(args, fmt);
    bytes = vsnprintf(buff, sizeof(buff), fmt, args);
    va_end(args);

    if (bytes >= sizeof(buff)) {
        /* output truncated */
        return -2;
    }

    /* register modules */
    if (pcaio_modepoll_use(2, &modepoll)) {
        return -2;
    }

    if (pcaio_modio_use(modepoll)) {
        return -2;
    }

    if (_sockpair(socks, &caddr)) {
        return -2;
    }

    DEBUG("caddr: %s", saddr2a(&caddr));
    tasks[0] = pcaio_task_new(_clientA, 3, socks[0], buff, bytes);
    tasks[1] = pcaio_task_new(connectionA, 3, &_chttpd, socks[1], &caddr);
    if ((tasks[0] == NULL) || (tasks[0] == NULL)) {
        ret = -2;
        goto done;
    }

    /* run event loop */
    ret = pcaio(1, tasks, 2);

done:
    pcaio_task_free(tasks[0]);
    pcaio_task_free(tasks[1]);
    close(socks[0]);
    close(socks[1]);
    return ret;
}
