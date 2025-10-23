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
#include <stdio.h>
#include <errno.h>

/* thirdparty */
#include <clog.h>
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>

/* local public */
#include "chttpd/chttpd.h"
#include "chttpd/addr.h"

/* local private */
#include "privatetypes.h"
#include "config.h"
#include "socket.h"
#include "connection.h"
#include "response.h"


const struct chttpd_config chttpd_defaultconfig = {
    .bind = "127.0.0.1:8080",
    .backlog = 10,
    .requestbuffer_mempages = 1,
    .connectionbuffer_mempages = 1,
    .connections_max = 10,
};


struct chttpd *
chttpd_new(const struct chttpd_config *c) {
    struct chttpd *s;

    s = malloc(sizeof(struct chttpd));
    if (s == NULL) {
        return NULL;
    }

    s->fd = -1;
    s->routescount = 0;
    s->config = c;
    return s;
}


void
chttpd_free(struct chttpd *s) {
    if (s == NULL) {
        return;
    }
    free(s);
}


int
chttpdA(int argc, void *argv[]) {
    struct chttpd *s = (struct chttpd *) argv[0];
    union saddr listenaddr;
    struct sockaddr_storage caddrbuff;
    union saddr *caddr = (union saddr *)&caddrbuff;
    socklen_t socklen;
    int cfd;

    s->fd = socket_bind(s->config->bind, &listenaddr);
    if (s->fd == -1) {
        return -1;
    }

    /* listen */
    if (listen(s->fd, s->config->backlog)) {
        return -1;
    }

    INFO("listening on: %s", saddr2a(&listenaddr));
    for (;;) {
        socklen = sizeof(caddrbuff);
        cfd = accept4A(s->fd, &caddr->sa, &socklen, SOCK_NONBLOCK);
        if (cfd == -1) {
            if (errno == ECONNABORTED) {
                /* ignore and continue */
                continue;
            }

            if ((errno == ENFILE) || (errno == EMFILE)) {
                /* open files limit, retry */
                WARN("open files linit reached");
                continue;
            }

            return -1;
        }

        pcaio_fschedule(connectionA, NULL, 3, s, cfd, caddr);
    }

    return 0;
}
