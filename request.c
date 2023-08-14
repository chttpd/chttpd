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
#include <clog.h>
#include <carrow.h>

#include "request.h"
#include "addr.h"


#undef CARROW_ENTITY
#define CARROW_ENTITY chttpd_request
#include <carrow_generic.c>  // NOLINT


void
requestA(struct chttpd_request_coro *self, struct chttpd_request *conn) {
    ssize_t bytes;
    struct mrb *buff = conn->reqbuff;
    CORO_START;
    static int e = 0;
    INFO("New conn: %s", sockaddr_dump(&conn->remoteaddr));

    while (true) {
        e = CET;

        /* tcp write */
        /* Write as mush as possible until EAGAIN */
        while (!mrb_isempty(buff)) {
            bytes = mrb_writeout(buff, conn->fd, mrb_used(buff));
            if ((bytes == -1) && CMUSTWAIT()) {
                e |= COUT;
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
            if ((bytes == -1) && CMUSTWAIT()) {
                e |= CIN;
                break;
            }
            if (bytes == -1) {
                CORO_REJECT("read(%d)", conn->fd);
            }
            if (bytes == 0) {
                CORO_REJECT("read(%d) EOF", conn->fd);
            }
        }

        /* reset errno and rewait events if neccessary */
        errno = 0;
        if (mrb_isempty(buff) || (e & COUT)) {
            CORO_WAIT(conn->fd, e);
        }
    }

    CORO_FINALLY;
    if (conn->fd != -1) {
        chttpd_request_evloop_unregister(conn->fd);
        close(conn->fd);
    }
    if (mrb_destroy(conn->reqbuff)) {
        ERROR("Cannot dispose buffers.");
    }
    if (mrb_destroy(conn->respbuff)) {
        ERROR("Cannot dispose buffers.");
    }
    free(conn);
    CORO_END;
}
