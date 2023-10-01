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


static ASYNC
requestA(struct caio_task *self, struct chttpd_connection *req) {
    int e = CAIO_ET;
    struct chttpd *chttpd = req->chttpd;
    CORO_START;

parse:
    /* Try to parse the request and perhapse wait for more data if the header
       is not transferred completely. */
    if (chttpd_request_parse(req)) {
        if (CORO_MUSTWAITFD()) {
            CORO_WAITFD(req->fd, e | CAIO_IN);
            /* Try again */
            goto parse;
        }
        req->closing = true;
        goto finally;
    }

    /* Begin request hook */
    if (chttpd->on_request_begin && chttpd->on_request_begin(req)) {
        /* Connection canceled by the hook */
        req->closing = true;
        goto finally;
    }

    /* Route(Find handler) */
    if (chttpd_route(req) == -1) {
        /* Raise 404 if default handler is not specified */
        if (chttpd->defaulthandler == NULL) {
            chttpd_response(req, "404 Not Found", "text/html", NULL);
            req->closing = true;

            /* Jump to flush, skip calling handler */
            goto flush;
        }

        /* Set to default handler */
        req->handler = chttpd->defaulthandler;
    }

    CORO_WAIT(req->handler, req);

flush:
    /* Flush the response */
    if (chttpd_response_flush(req)) {
        if (CORO_MUSTWAITFD()) {
            CORO_WAITFD((req)->fd, e | CAIO_OUT);
            goto flush;
        }
        req->closing = true;
    }

    /* End request hook */
    if (chttpd->on_request_end && chttpd->on_request_end(req)) {
        /* Connection canceled by the hook */
        req->closing = true;
    }

finally:
    CORO_FINALLY;
    /* free request resources */
    chttpd_request_reset(req);

    /* Keep open the connection, and become ready for the next request */
    if ((!req->closing) && (req->connection == HTTP_CT_KEEPALIVE)) {
        goto parse;
    }

    /* close connection hook */
    if (chttpd->on_connection_close && chttpd->on_connection_close(
                chttpd, req->fd, req->remoteaddr)) {
        /* connection closing has canceled by the hook */
        goto parse;
    }

    /* free connection resources */
    chttpd_connection_dispose(req);
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

        /* New connection hook */
        if (chttpd->on_connection_open &&
                chttpd->on_connection_open(chttpd, reqfd, connaddr)) {
            /* Connection canceled by the hook */
            close(reqfd);
            continue;
        }

        /* New Connection */
        req = chttpd_connection_new(chttpd, reqfd, connaddr);
        if (req == NULL) {
            /* Out of memory or calcelled by the hook, retrying next. */
            close(reqfd);
            continue;
        }

        CAIO_RUN(requestA, req);
    }

    CORO_FINALLY;
    caio_evloop_unregister(chttpd->listenfd);
    close(chttpd->listenfd);
    chttpd_router_cleanup(chttpd->routes);
}


void
chttpd_defaults(struct chttpd *restrict chttpd) {
    int pagesize = getpagesize();

    /* Socket */
    chttpd->bindaddr = "0.0.0.0";
    chttpd->bindport = 8080;

    /* Limits */
    chttpd->backlog = 10;
    chttpd->maxconn = 100;
    chttpd->request_buffsize = CHTTPD_REQUEST_DEFAULT_BUFFSIZE(pagesize);
    chttpd->response_buffsize = CHTTPD_RESPONSE_DEFAULT_BUFFSIZE(pagesize);

    /* Route */
    chttpd->routes = NULL;
    chttpd->defaulthandler = NULL;

    /* Hooks */
    chttpd->on_connection_open = NULL;
    chttpd->on_connection_close = NULL;
    chttpd->on_request_begin = NULL;
    chttpd->on_request_end = NULL;
}


int
chttpd_forever(struct chttpd *restrict chttpd) {
    int pagesize = getpagesize();

    if (chttpd->request_buffsize % pagesize) {
        ERROR("Invalid request buffer size: %lu, it should be multiple of "
            "pagesize (%d), see getpagesize(2).", chttpd->request_buffsize,
            pagesize);
        return -1;
    }

    if (chttpd->response_buffsize % pagesize) {
        ERROR("Invalid response buffer size: %lu, it should be multiple of "
            "pagesize (%d), see getpagesize(2).", chttpd->response_buffsize,
            pagesize);
        return -1;
    }

    return CAIO(chttpdA, chttpd, chttpd->maxconn + 1);
}
