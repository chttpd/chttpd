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


/**
 * return the number of characters which would have been written to the target
 * buffer if enought space had been available. otherwise, chunksize and -1 for
 * error.
 */
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

    /* read and search for the first CRLF */
    headlen = chttpd_connection_readsearchA(c, "\r\n", 5);
    if (headlen <= 0) {
        return -1;
    }

    /* parse the chunksize (haxdigit) */
    in = mrb_readerptr(&c->ring);
    errno = 0;
    chunksize = strtol(in, &endptr, 16);
    if (errno == ERANGE) {
        /* value out of range */
        return -1;
    }

    if (chunksize == 0) {
        if (in[0] == '0') {
            /* termination chunk */
            return 0;
        }

        /* parse error */
        return -1;
    }

    /* mark the readsize + sizeof(CRLF) as read */
    if (mrb_skip(&c->ring, headlen + 2)) {
        return -1;
    }

    /* check the target buffer has enought space to hold the chunk */
    if (chunksize > max) {
        /* return the number of characters which would have been written to
           the target buffer if enought space had been available. */
        return chunksize;
    }

    /* ensure the whole chunk(including CRLF suffix) is received */
    if (chttpd_connection_atleastA(c, chunksize + 2)) {
        return -1;
    }

    /* ensure the trailing CRLF */
    in = mrb_readerptr(&c->ring) + chunksize;
    if ((in[0] != '\r') || (in[1] != '\n')) {
        return -1;
    }

    /* copy the chunk without the trailing CRLF*/
    bytes = mrb_get(&c->ring, buff, chunksize);
    if (bytes != chunksize) {
        return -1;
    }

    /* mark the trailing CRLF as read */
    if (mrb_skip(&c->ring, 2)) {
        return -1;
    }

    return chunksize;
}
