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

/* local public */
#include "chttpd/chttpd.h"

/* local private */
#include "request.h"


ssize_t
chttpd_request_readchunkA(struct chttpd_connection *c, char *buff,
        size_t max) {
    ssize_t bytes;
    ssize_t chunksize;
    ssize_t headlen;
    char *in;
    char *endptr;

    if (buff == NULL) {
        return -1;
    }

    /* read as mush as possible into the ringbuffer */
    bytes = chttpd_connection_readallA(c);
    if (bytes < 5) {
        return -1;
    }

    /* search for the first CRLF */
    headlen = chttpd_connection_search(c, "\r\n");
    if (headlen <= 0) {
        return -1;
    }

    /* parse the chunksize (haxdigit) */
    in = mrb_readerptr(&c->ring);
    errno = 0;
    chunksize = strtol(in, &endptr, 16);
    if ((chunksize == 0) || (errno == ERANGE)) {
        return -1;
    }

    /* mark the readsize + sizeof(CRLF) as read */
    if (mrb_skip(&c->ring, headlen + 2)) {
        return -1;
    }

    /* copy the chunk */
    bytes = mrb_getmin(&c->ring, buff, chunksize, max);
    if (bytes != chunksize) {
        return -1;
    }

    /* mark the chunksize as read */
    if (mrb_skip(&c->ring, chunksize)) {
        return -1;
    }

    /* ensure the trailing CRLF */
    in = mrb_readerptr(&c->ring);
    if ((in[0] != '\r') || (in[1] != '\n')) {
        return -1;
    }

    /* mark the trailing CRLF as read */
    if (mrb_skip(&c->ring, 2)) {
        return -1;
    }

    return chunksize;
}
