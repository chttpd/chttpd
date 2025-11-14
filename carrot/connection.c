// Copyright 2025 Vahid Mardani
/*
 * This file is part of carrot.
 *  carrot is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  carrot is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with carrot. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Author: Vahid Mardani <vahid.mardani@gmail.com>
 */
/* standard */
#include <errno.h>
#include <string.h>

/* thirdparty */
#include <mrb.h>
#include <chttp/chttp.h>
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>

/* local public */
#include "carrot/connection.h"

/* local private */
#include "common.h"


/** read as much as possible from the peer and returns length of the newly
 * read data, -2 when buffer is full, 0 on end-of-file and -1 on error.
 * The out ptr will set to the start of the received data on successfull read.
 */
int
carrot_connection_recvallA(struct carrot_connection *c, char **out) {
    ssize_t bytes;
    char *start = mrb_readerptr(&c->ring);
    pcaio_relaxA(0);

retry:
    bytes = mrb_readallin(&c->ring, c->fd);
    if (bytes == -2) {
        return -2;
    }

    if (bytes == -1) {
        if (!RETRY(errno)) {
            return -1;
        }

        if (pcaio_modio_await(c->fd, IOIN)) {
            return -1;
        }

        errno = 0;
        goto retry;
    }

    if (out) {
        *out = start;
    }
    return bytes;
}


/** wait until read error, buffer become full or find the s inside the input
 * buffer.
 * search inside the circular buffer for the given expression.
 * returns:
 *  0: EOF and not found
 * -1: read error
 * -2: input buffer full and not found
 * -3: s is empty or NULL
 *  n: length of found string.
 */
ssize_t
carrot_connection_recvsearchA(struct carrot_connection *c, const char *s) {
    size_t avail;
    char *chunk = mrb_readerptr(&c->ring);
    ssize_t chunksize = mrb_used(&c->ring);
    int slen;
    char *found;
    int lookbehind;

    if (s == NULL) {
        return -3;
    }

    slen = strlen(s);
    if (slen == 0) {
        return -3;
    }

    if (chunksize) {
        found = memmem(chunk, chunksize, s, slen);
        if (found) {
            return found - chunk;
        }
    }

    while ((avail = mrb_available(&c->ring))) {
        lookbehind = MIN(mrb_used(&c->ring), slen - 1);
        chunksize = carrot_connection_recvallA(c, &chunk);
        if (chunksize < 0) {
            return chunksize;
        }

        if (chunksize && (chunksize < slen)) {
            continue;
        }

        found = memmem(chunk - lookbehind, chunksize, s, slen);
        if (found) {
            return found - mrb_readerptr(&c->ring);
        }

        if (chunksize == 0) {
            return 0;
        }
    }

    return -2;
}


/**
 * return the number of characters which would have been written to the target
 * buffer and -2 if enought space had been available. otherwise, chunksize and
 * -1 for error.
 */
ssize_t
carrot_connection_recvchunkA(struct carrot_connection *c, const char **start) {
    ssize_t chunksize;
    size_t garbage;
    char *in = mrb_readerptr(&c->ring);
    size_t inlen = mrb_used(&c->ring);
    ssize_t ret;

retry:
    chunksize = chttp_chunked_parse(in, inlen, start, &garbage);
    if (chunksize == 0) {
        return 0;
    }

    if (chunksize == -1) {
        return -1;
    }

    if (chunksize == -2) {
        /* more data needed */
        ret = carrot_connection_recvallA(c, NULL);
        if (ret <= 0) {
            return  ret;
        }

        inlen += ret;
        goto retry;
    }

    mrb_skip(&c->ring, garbage);
    return chunksize;
}


ssize_t
carrot_connection_sendpacketA(struct carrot_connection *c,
        struct chttp_packet *p) {
    struct iovec v[4];
    int vcount = sizeof(v) / sizeof(struct iovec);
    size_t totallen;
    size_t written;

    totallen = chttp_packet_iovec(p, v, &vcount);
    written = writevA(c->fd, v, vcount);
    if (written != totallen) {
        // TODO: write the rest of the buffer later after pcaio_relaxA
        return -1;
    }

    chttp_packet_reset(p);
    return totallen;
}
