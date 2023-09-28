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


void
requestA(struct caio_task *self, struct chttpd_connection *req) {
    char *header;
    ssize_t headerlen;
    int seplen;
    CORO_START;

    if (req->status == CCS_REQUEST_HEADER) {
        /* Check the whole header is available or not */
        headerlen = mrb_search(req->inbuff, "\r\n\r\n", 4, 0,
                CHTTPD_HEADERSIZE);
        if (headerlen == -1) {
            // TODO: Preserve searched area to improve performance.
            if (mrb_used(req->inbuff) >= CHTTPD_HEADERSIZE) {
                req->status = CCS_CLOSING;
            }
            CORO_RETURN;
        }

        /* Pick the first CRLF to be able to parse the last header. */
        headerlen += 2;

        /* Allocate memory for request header */
        req->header = malloc(headerlen + 1);
        if (req->header == NULL) {
            req->status = CCS_CLOSING;
            CORO_RETURN;
        }
        req->headerlen = headerlen;

        /* Read whole HTTP header from the request buffer */
        mrb_get(req->inbuff, req->header, headerlen);

        /* Skip the second CRLF */
        mrb_skip(req->inbuff, 2);

        /* Parse the request */
        if (chttpd_request_parse(req)) {
            /* Request parse error */
            req->status = CCS_CLOSING;
            CORO_RETURN;
        }

        /* Route(Find handler) */
        if (chttpd_route(req)) {
            /* Raise 404 if default handler is not specified */
            if (req->chttpd->defaulthandler == NULL) {
                chttpd_response(req, "404 Not Found");
                req->status = CCS_CLOSING;
                CORO_RETURN;
            }

            /* Set to default handler */
            req->handler = req->chttpd->defaulthandler;
        }
    }

    // TODO: call handler

    CORO_FINALLY;
    chttpd_connection_reset(req);
}


ASYNC
connectionA(struct caio_task *self, struct chttpd_connection *conn) {
    ssize_t bytes;
    struct mrb *inbuff = conn->inbuff;
    struct mrb *outbuff = conn->outbuff;
    CORO_START;
    static int e = 0;
    INFO("new connection: %s", sockaddr_dump(&conn->remoteaddr));

    while (true) {
        e = CAIO_ET;

        /* sock write */
        /* Write as mush as possible until EAGAIN */
        while (!mrb_isempty(outbuff)) {
            bytes = mrb_writeout(outbuff, conn->fd, mrb_used(outbuff));
            if ((bytes == -1) && CORO_MUSTWAITFD()) {
                e |= CAIO_OUT;
                break;
            }
            if (bytes == -1) {
                CORO_REJECT("write(%d)", conn->fd);
            }
            if (bytes == 0) {
                CORO_REJECT("write(%d) EOF", conn->fd);
            }
        }

        /* sock read */
        /* Read as mush as possible until EAGAIN */
        while ((conn->status != CCS_CLOSING) && (!mrb_isfull(inbuff))) {
            bytes = mrb_readin(inbuff, conn->fd, mrb_available(inbuff));
            if ((bytes == -1) && CORO_MUSTWAITFD()) {
                e |= CAIO_IN;
                break;
            }
            if (bytes == -1) {
                CORO_REJECT("read(%d)", conn->fd);
            }
            if (bytes == 0) {
                CORO_REJECT("read(%d) EOF", conn->fd);
            }
        }

        if (conn->status != CCS_CLOSING) {
            CORO_WAIT(requestA, conn);
        }

        if ((conn->status == CCS_CLOSING) && mrb_isempty(outbuff)) {
            break;
        }

    waitfd:
        /* reset errno and rewait events if neccessary */
        errno = 0;
        if (!mrb_isfull(inbuff)) {
            e |= CAIO_IN;
        }

        if (e != CAIO_ET) {
            CORO_WAITFD(conn->fd, e);
        }
    }

    CORO_FINALLY;
    if (conn->fd != -1) {
        caio_evloop_unregister(conn->fd);
        close(conn->fd);
    }
    if (mrb_destroy(conn->inbuff)) {
        ERROR("Cannot dispose buffers.");
    }
    if (mrb_destroy(conn->outbuff)) {
        ERROR("Cannot dispose buffers.");
    }
    free(conn);
}


ASYNC
chttpdA(struct caio_task *self, struct chttpd *chttpd) {
    socklen_t addrlen = sizeof(struct sockaddr);
    struct sockaddr connaddr;
    int connfd;
    CORO_START;

    if (chttpd_router_compilepatterns(chttpd->routes)) {
        CORO_REJECT("Route pattern error");
    }

    chttpd->listenfd = chttpd_listen(chttpd);
    if (chttpd->listenfd == -1) {
        CORO_REJECT(NULL);
    }

    while (true) {
        connfd = accept4(chttpd->listenfd, &connaddr, &addrlen, SOCK_NONBLOCK);
        if ((connfd == -1) && CORO_MUSTWAITFD()) {
            CORO_WAITFD(chttpd->listenfd, CAIO_IN | CAIO_ET);
            continue;
        }

        if (connfd == -1) {
            CORO_REJECT("accept4");
        }

        /* New Connection */
        struct chttpd_connection *conn = chttpd_connection_new(chttpd, connfd,
                connaddr);
        if (conn == NULL) {
            CORO_REJECT("Out of memory");
        }
        CAIO_RUN(connectionA, conn);
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
