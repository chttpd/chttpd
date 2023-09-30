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
#include "networking.h"
#include "connection.h"
#include "request.h"
#include "router.h"
#include "response.h"


ASYNC
requestA(struct caio_task *self, struct chttpd_connection *req) {
    int e = CAIO_ET;
    CORO_START;
    INFO("new connection: %s", sockaddr_dump(&req->remoteaddr));

parse:
    if (chttpd_request_parse(req)) {
        if (CORO_MUSTWAITFD()) {
            CORO_WAITFD(req->fd, e | CAIO_IN);
            goto parse;
        }
        req->closing = true;
        CORO_RETURN;
    }

    DEBUG("routing: %s", req->path);
    /* Route(Find handler) */
    if (chttpd_route(req) == -1) {
        /* Raise 404 if default handler is not specified */
        if (req->chttpd->defaulthandler == NULL) {
            chttpd_response(req, "404 Not Found", "text/html", NULL);
            req->closing = true;
            CORO_REJECT("Cannot find handler");
        }

        /* Set to default handler */
        req->handler = req->chttpd->defaulthandler;
    }

    CORO_WAIT(req->handler, req);
    CHTTPD_RESPONSE_FLUSH(req);
    CORO_FINALLY;

    chttpd_response_flush(req);
    if (!(req->closing)) {
        chttpd_request_reset(req);
        goto parse;
    }

    if (req->fd != -1) {
        caio_evloop_unregister(req->fd);
        close(req->fd);
    }
    if (mrb_destroy(req->inbuff)) {
        ERROR("Cannot dispose buffers.");
    }
    if (mrb_destroy(req->outbuff)) {
        ERROR("Cannot dispose buffers.");
    }
    free(req);
}


ASYNC
chttpdA(struct caio_task *self, struct chttpd *chttpd) {
    socklen_t addrlen = sizeof(struct sockaddr);
    struct sockaddr connaddr;
    int reqfd;
    struct chttpd_connection *req;
    CORO_START;

    if (chttpd_router_compilepatterns(chttpd->routes)) {
        CORO_REJECT("Route pattern error");
    }

    chttpd->listenfd = chttpd_listen(chttpd);
    if (chttpd->listenfd == -1) {
        CORO_REJECT(NULL);
    }

    while (true) {
        reqfd = accept4(chttpd->listenfd, &connaddr, &addrlen, SOCK_NONBLOCK);
        if ((reqfd == -1) && CORO_MUSTWAITFD()) {
            CORO_WAITFD(chttpd->listenfd, CAIO_IN | CAIO_ET);
            continue;
        }

        if (reqfd == -1) {
            CORO_REJECT("accept4");
        }

        /* New Connection */
        req = chttpd_connection_new(chttpd, reqfd, connaddr);
        if (req == NULL) {
            CORO_REJECT("Out of memory");
        }
        CAIO_RUN(requestA, req);
    }

    CORO_FINALLY;
    caio_evloop_unregister(chttpd->listenfd);
    close(chttpd->listenfd);
    chttpd_router_cleanup(chttpd->routes);
}


int
chttpd_forever(struct chttpd *restrict chttpd) {
    return CAIO(chttpdA, chttpd, chttpd->maxconn + 1);
}
