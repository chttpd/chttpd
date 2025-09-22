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
#include <errno.h>

/* thirdparty */
#include <clog.h>
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>

/* local private */
#include "config.h"
#include "socket.h"
#include "connection.h"

/* local public */
#include "chttpd/chttpd.h"
#include "chttpd/addr.h"


struct route {
    const char *verb;
    const char *path;
    chttpd_handler_t handler;
    void *ptr;
};


struct chttpd {
    struct chttpd_config *config;

    /* bind file */
    int fd;

    /* routes */
    unsigned char routescount;
    struct route routes[CONFIG_CHTTPD_ROUTES_MAX];
};


static struct chttpd_config _defaultconfig = {
    .bind = "127.0.0.1:8080",
    .backlog = 10,
};


void
chttpd_config_default(struct chttpd_config *c) {
    memcpy(c, &_defaultconfig, sizeof(_defaultconfig));
}


struct chttpd *
chttpd_new(struct chttpd_config *c) {
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
chttpd_route(struct chttpd *s, const char *verb, const char *path,
        chttpd_handler_t handler, void *ptr) {
    struct route *r;

    if (s->routescount >= CONFIG_CHTTPD_ROUTES_MAX) {
        return -1;
    }

    r = &s->routes[s->routescount++];
    r->verb = verb;
    r->path = path;
    r->handler = handler;
    r->ptr = ptr;
    return 0;
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

        pcaio_fschedule(connectionA, 3, s, cfd, caddr);
    }

    return 0;
}
