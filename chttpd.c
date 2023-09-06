// Copyright 2023 Vahid Mardani
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
#include <unistd.h>
#include <sys/socket.h>

#include <clog.h>
#include <mrb.h>
#include <caio.h>

#include "chttpd.h"
#include "request.h"
#include "connection.h"
#include "addr.h"


ASYNC
chttpdA(struct caio_task *self, struct chttpd *state) {
    socklen_t addrlen = sizeof(struct sockaddr);
    struct sockaddr bindaddr;
    struct sockaddr connaddr;
    static int fd;
    int connfd;
    int res;
    int option = 1;
    CORO_START;

    /* Parse listen address */
    sockaddr_parse(&bindaddr, state->bindaddr, state->bindport);

    /* Create socket */
    fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    /* Allow reuse the address */
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    /* Bind to tcp port */
    res = bind(fd, &bindaddr, sizeof(bindaddr));
    if (res) {
        CORO_REJECT("Cannot bind on: %s", sockaddr_dump(&bindaddr));
    }

    /* Listen */
    res = listen(fd, state->backlog);
    INFO("Listening on: %s", sockaddr_dump(&bindaddr));
    if (res) {
        CORO_REJECT("Cannot listen on: %s", sockaddr_dump(&bindaddr));
    }

    while (true) {
        connfd = accept4(fd, &connaddr, &addrlen, SOCK_NONBLOCK);
        if ((connfd == -1) && CORO_MUSTWAITFD()) {
            CORO_WAITFD(fd, CAIO_IN | CAIO_ET);
            continue;
        }

        if (connfd == -1) {
            CORO_REJECT("accept4");
        }

        /* New Connection */
        struct chttpd_connection *c = malloc(
                sizeof(struct chttpd_connection));
        if (c == NULL) {
            CORO_REJECT("Out of memory");
        }

        c->fd = connfd;
        c->localaddr = bindaddr;
        c->remoteaddr = connaddr;
        c->reqbuff = mrb_create(state->buffsize);
        c->respbuff = mrb_create(state->buffsize);
        CAIO_RUN(connectionA, c);
    }

    CORO_FINALLY;
    caio_evloop_unregister(fd);
    close(fd);
}


int
chttpd_forever(struct chttpd *state, int maxconn) {
    return CAIO(chttpdA, state, maxconn + 1);
}
