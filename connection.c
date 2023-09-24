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

#include <clog.h>
#include <caio.h>

#include "chttpd.h"
#include "request.h"
#include "connection.h"
#include "addr.h"


int
chttpd_connection_close(struct chttpd_request *req) {
    return -1;
}


ASYNC
connectionA(struct caio_task *self, struct chttpd_request *req) {
    ssize_t bytes;
    struct mrb *inbuff = req->inbuff;
    struct mrb *outbuff = req->outbuff;
    CORO_START;
    static int e = 0;
    INFO("new connection: %s", sockaddr_dump(&req->remoteaddr));

    while (true) {
        e = CAIO_ET;

        /* tcp write */
        /* Write as mush as possible until EAGAIN */
        while (!mrb_isempty(outbuff)) {
            bytes = mrb_writeout(outbuff, req->fd, mrb_used(outbuff));
            if ((bytes == -1) && CORO_MUSTWAITFD()) {
                e |= CAIO_OUT;
                break;
            }
            if (bytes == -1) {
                CORO_REJECT("write(%d)", req->fd);
            }
            if (bytes == 0) {
                CORO_REJECT("write(%d) EOF", req->fd);
            }
        }

        /* tcp read */
        /* Read as mush as possible until EAGAIN */
        while ((req->status != CCS_CLOSING) && (!mrb_isfull(inbuff))) {
            bytes = mrb_readin(inbuff, req->fd, mrb_available(inbuff));
            if ((bytes == -1) && CORO_MUSTWAITFD()) {
                e |= CAIO_IN;
                break;
            }
            if (bytes == -1) {
                CORO_REJECT("read(%d)", req->fd);
            }
            if (bytes == 0) {
                CORO_REJECT("read(%d) EOF", req->fd);
            }
        }

        if (req->status != CCS_CLOSING) {
            CORO_WAIT(requestA, req);
        }

        if ((req->status == CCS_CLOSING) && mrb_isempty(outbuff)) {
            break;
        }

    waitfd:
        /* reset errno and rewait events if neccessary */
        errno = 0;
        if (!mrb_isfull(inbuff)) {
            e |= CAIO_IN;
        }

        if (e != CAIO_ET) {
            CORO_WAITFD(req->fd, e);
        }
    }

    CORO_FINALLY;
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
