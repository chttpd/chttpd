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

/* thirdparty */
#include <clog.h>
#include <chttp.h>
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>

/* local public */
#include "chttpd/chttpd.h"

/* local private */
#include "response.h"


int
response_tofileA(struct chttp_response *resp, int fd) {
    struct iovec v[2];
    size_t totallen = resp->headerlen + resp->contentlength;
    size_t written;

    v[0].iov_base = (void *)resp->header;
    v[0].iov_len = resp->headerlen;
    v[1].iov_base = resp->content;
    v[1].iov_len = resp->contentlength;

    written = writevA(fd, v, 2);
    if (written != totallen) {
        // TODO: write the rest of the buffer later after pcaio_relaxA
        return -1;
    }

    return totallen;
}
