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


ASYNC
connectionA(struct caio_task *self, struct chttpd_request *req) {
    /*
    TODO:
    - Wait for headers, then Parse them
    - Find handler, otherwise 404
    */
    ssize_t bytes;
    struct mrb *buff = req->inbuff;
    CORO_START;
    static int e = 0;
    INFO("New req: %s", sockaddr_dump(&req->remoteaddr));

    while (true) {
        e = CAIO_ET;

        /* tcp write */
        /* Write as mush as possible until EAGAIN */
        while (!mrb_isempty(buff)) {
            bytes = mrb_writeout(buff, req->fd, mrb_used(buff));
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
        while (!mrb_isfull(buff)) {
            bytes = mrb_readin(buff, req->fd, mrb_available(buff));
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

        CORO_WAIT(requestA, req);
        if (req->status == CCS_CLOSING) {
            break;
        }

    waitfd:
        /* reset errno and rewait events if neccessary */
        errno = 0;
        if (mrb_isempty(buff) || (e & CAIO_OUT)) {
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
