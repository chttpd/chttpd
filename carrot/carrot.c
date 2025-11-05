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
#include <errno.h>

/* thirdparty */
#include <clog.h>
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>

/* local public */
#include "carrot/carrot.h"
#include "carrot/addr.h"

/* local private */
#include "common.h"
#include "config.h"
#include "socket.h"
#include "router.h"
#include "connection.h"


struct carrot *
carrot_new(const struct carrot_config *c) {
    struct carrot *s;

    s = malloc(sizeof(struct carrot));
    if (s == NULL) {
        return NULL;
    }

    s->listenfd = -1;
    s->router.count = 0;
    s->config = c;
    return s;
}


void
carrot_free(struct carrot *s) {
    if (s == NULL) {
        return;
    }
    free(s);
}


int
carrot_route(struct carrot *s, const char *verb, const char *path,
        carrot_handler_t handler, void *ptr) {
    return router_append(&s->router, verb, path, handler, ptr);
}


int
carrotA(int argc, void *argv[]) {
    struct carrot *s = (struct carrot *) argv[0];
    union saddr listenaddr;
    struct sockaddr_storage caddrbuff;
    union saddr *caddr = (union saddr *)&caddrbuff;
    socklen_t socklen;
    int cfd;

    s->listenfd = socket_bind(s->config->bind, &listenaddr);
    if (s->listenfd == -1) {
        return -1;
    }

    /* listen */
    if (listen(s->listenfd, s->config->backlog)) {
        return -1;
    }

    INFO("listening on: %s", saddr2a(&listenaddr));
    for (;;) {
        socklen = sizeof(caddrbuff);
        cfd = accept4A(s->listenfd, &caddr->sa, &socklen, SOCK_NONBLOCK);
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

    // TODO: move it to pcaio task disposation callback
    close(s->listenfd);
    return 0;
}
