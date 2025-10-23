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
response_tofileA(struct chttp_responsemaker *resp, int fd) {
    struct iovec v[3];
    size_t totallen = resp->headerlen + resp->contentlength;
    size_t written;

    v[0].iov_base = (void *)resp->header;
    v[0].iov_len = resp->headerlen;
    v[1].iov_base = "\r\n";
    v[1].iov_len = 2;
    v[2].iov_base = resp->content;
    v[2].iov_len = resp->contentlength;

    written = writevA(fd, v, 2);
    if (written != totallen) {
        // TODO: write the rest of the buffer later after pcaio_relaxA
        return -1;
    }

    return totallen;
}


int
chttpd_responseA(struct chttpd_connection *c, int status, const char *text) {
    int contentlen;

    if (text == NULL) {
        text = chttp_status_text(status);
    }

    if (chttp_responsemaker_start(c->request, status, text)) {
        return -1;
    }

    if (chttp_responsemaker_contenttype(c->request, "text/plain", "utf-8")) {
        return -1;
    }

    // TODO: config the content size
    if (chttp_responsemaker_content_allocate(c->request, 512)) {
        return -1;
    }

    contentlen = chttp_responsemaker_content_write(c->request,
            "%d %s\r\n", status, text);

    DEBUG("content len: %d", contentlen);
    return response_tofileA(&c->request->response, c->fd);
}
