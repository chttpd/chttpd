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

#include <caio.h>

#include "chttpd.h"
#include "connection.h"


struct chttpd_connection*
chttpd_connection_new(struct chttpd *chttpd, int fd, struct sockaddr addr) {
    struct chttpd_connection *conn;

    conn = malloc(sizeof(struct chttpd_connection));
    if (conn == NULL) {
        ERROR("Out of memory when allocating new connection.");
        return NULL;
    }
    memset(conn, 0, sizeof(struct chttpd_connection));

    conn->chttpd = chttpd;
    conn->fd = fd;
    conn->remoteaddr = addr;
    conn->inbuff = mrb_create(chttpd->request_buffsize);
    if (conn->inbuff == NULL) {
        goto failed;
    }

    conn->outbuff = mrb_create(chttpd->response_buffsize);
    if (conn->outbuff == NULL) {
        goto failed;
    }

    return conn;

failed:
    free(conn);
    return NULL;
}


void
chttpd_connection_dispose(struct chttpd_connection *req) {
    if (req == NULL) {
        return;
    }

    if (req->fd != -1) {
        caio_evloop_unregister(req->fd);
        close(req->fd);
    }
    if (mrb_destroy(req->inbuff) || mrb_destroy(req->outbuff)) {
        ERROR("Cannot dispose buffer(s).");
    }
    free(req);
}
