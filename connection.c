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
connectionA(struct caio_task *self, struct chttpd_connection *conn) {
    /*
    TODO:
    - Wait for headers, then Parse them
    - Find handler, otherwise 404
    */
    ssize_t bytes;
    struct mrb *buff = conn->inbuff;
    CORO_START;
    static int e = 0;
    INFO("New conn: %s", sockaddr_dump(&conn->remoteaddr));

    while (true) {
        e = CAIO_ET;

        /* tcp write */
        /* Write as mush as possible until EAGAIN */
        while (!mrb_isempty(buff)) {
            bytes = mrb_writeout(buff, conn->fd, mrb_used(buff));
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

        /* tcp read */
        /* Read as mush as possible until EAGAIN */
        while (!mrb_isfull(buff)) {
            bytes = mrb_readin(buff, conn->fd, mrb_available(buff));
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

        CORO_WAIT(requestA, conn->request);
        if (conn->status == CCS_HEADER) {
            conn->status == CCS_BODY;
        }
        else if (conn->status == CCS_CLOSING) {
            free(conn->request);
            break;
        }
        else if (conn->status == CCS_COMPLETED) {
            free(conn->request);
        }

    waitfd:
        /* reset errno and rewait events if neccessary */
        errno = 0;
        if (mrb_isempty(buff) || (e & CAIO_OUT)) {
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
