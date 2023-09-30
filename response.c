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
#include "chttpd.h"
#include "response.h"
#include "manifest.h"


int
chttpd_response_flush(struct chttpd_connection *req) {
    ssize_t bytes;

    /* sock write */
    /* Write as mush as possible until EAGAIN */
    while (!mrb_isempty(req->outbuff)) {
        bytes = mrb_writeout(req->outbuff, req->fd, mrb_used(req->outbuff));
        if (bytes <= 0) {
            return -1;
        }
    }

    return 0;
}


ssize_t
chttpd_response_print(struct chttpd_connection *req, const char *format,
        ...) {
    va_list args;

    if (format) {
        va_start(args, format);
    }

    ssize_t written = mrb_vprint(req->outbuff, format, args);

    if (format) {
        va_end(args);
    }

    return written;
}


ssize_t
chttpd_response_start(struct chttpd_connection *req, const char *status) {
    ssize_t bytes;

    if (status == NULL) {
        return -1;
    }

    bytes = chttpd_response_print(req,
            "HTTP/1.1 %s\r\n"
            "Server: chttpd/" CHTTPD_VERSION "\r\n",
            status);
    if (bytes <= 0) {
        return -1;
    }

    return bytes;
}


ssize_t
chttpd_response(struct chttpd_connection *req, const char *restrict status,
        const char *format, ...) {
    va_list args;
    ssize_t written = 0;
    ssize_t bytes;
    ssize_t contentlen = 0;
    char *tmp;

    bytes = chttpd_response_start(req, status);
    if (bytes <= 0) {
        return -1;
    }
    written += bytes;

    tmp = malloc(CHTTPD_RESPONSE_BODY_MAXLEN + 1);
    if (tmp == NULL) {
        return -1;
    }

    if (format) {
        va_start(args, format);
        contentlen = vsprintf(tmp, format, args);
        va_end(args);
        if (contentlen <= 0) {
            free(tmp);
            return -1;
        }
        tmp[contentlen] = 0;
    }

    bytes = chttpd_response_print(req, "Content-Length: %d\r\n\r\n",
            contentlen);
    if (bytes <= 0) {
        free(tmp);
        return -1;
    }
    written += bytes;

    if (contentlen) {
        bytes = chttpd_response_write(req, tmp, contentlen);
        if (bytes == -1) {
            free(tmp);
            return -1;
        }
        written += contentlen;

        bytes = chttpd_response_write(req, "\r\n", 2);
        if (bytes == -1) {
            free(tmp);
            return -1;
        }
        written += bytes;
    }
    free(tmp);
    return written;
}
