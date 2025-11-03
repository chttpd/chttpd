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
 * buffer and -2 if enought space had been available. otherwise, chunksize and
 * -1 for error.
 */
ssize_t
chttpd_request_readchunkA(struct chttpd_connection *c, const char **start) {
    ssize_t chunksize;
    size_t garbage;
    char *in = mrb_readerptr(&c->ring);
    size_t inlen = mrb_used(&c->ring);
    ssize_t ret;

retry:
    chunksize = chttp_chunkedcodec_getchunk(in, inlen, start, &garbage);
    if (chunksize == 0) {
        return 0;
    }

    if (chunksize == -1) {
        return -1;
    }

    if (chunksize == -2) {
        /* more data needed */
        ret = connection_readallA(c, NULL);
        if (ret <= 0) {
            return  ret;
        }

        inlen += ret;
        goto retry;
    }

    mrb_skip(&c->ring, garbage);
    return chunksize;
}
