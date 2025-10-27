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
#include <stdio.h>
#include <stdarg.h>

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
chttpd_response_start(struct chttpd_connection *c, int status,
        const char *text) {
    return chttp_responsemaker_start(c->request, status, text);
}


int
chttpd_response_contenttype(struct chttpd_connection *c, const char *type,
        const char *charset) {
    return chttp_responsemaker_contenttype(c->request, type, charset);
}


int
chttpd_response_header(struct chttpd_connection *c, const char *fmt, ...) {
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = chttp_responsemaker_vheader(c->request, fmt, args);
    va_end(args);

    return ret;
}


int
chttpd_response_header_close(struct chttpd_connection *c) {
    return chttp_responsemaker_header_close(c->request);
}


int
chttpd_response_allocate(struct chttpd_connection *c, size_t size) {
    return chttp_responsemaker_content_allocate(c->request, size);
}


ssize_t
chttpd_response_write(struct chttpd_connection *c, const char *fmt, ...) {
    int ret;
    va_list args;

    va_start(args, fmt);
    ret = chttp_responsemaker_content_vwrite(c->request, fmt, args);
    va_end(args);

    return ret;
}


int
chttpd_responseA(struct chttpd_connection *c, int status, const char *text,
        const char *content, size_t contentlen) {
    if (text == NULL) {
        text = chttp_status_text(status);
    }

    if (chttpd_response_start(c, status, text)) {
        return -1;
    }

    if (chttpd_response_contenttype(c, "text/plain", "utf-8")) {
        return -1;
    }

    if (chttpd_response_allocate(c, contentlen)) {
        return -1;
    }

    if (chttpd_response_write(c, content) == -1) {
        return -1;
    }

    if (chttpd_response_header_close(c)) {
        return -1;
    }

    return chttpd_response_flushA(c);
}


int
chttpd_response_errorA(struct chttpd_connection *c, int status,
        const char *text) {
    const char *content;

    content = chttp_status_text(status);
    return chttpd_responseA(c, status, text, content, strlen(content));
}


ssize_t
chttpd_response_flushA(struct chttpd_connection *c) {
    int vectors = 1;
    struct iovec v[3];
    struct chttp_responsemaker *resp = &c->request->response;
    size_t totallen;
    size_t written;

    v[0].iov_base = (void *)resp->header;
    v[0].iov_len = resp->headerlen;
    totallen = resp->headerlen;
    if (resp->content) {
        v[1].iov_base = resp->content;
        v[1].iov_len = resp->contentlength;
        vectors++;
        totallen += resp->contentlength;
    }

    written = writevA(c->fd, v, vectors);
    if (written != totallen) {
        // TODO: write the rest of the buffer later after pcaio_relaxA
        return -1;
    }

    return totallen;
}


ssize_t
chttpd_response_header_flushA(struct chttpd_connection *c) {
    struct chttp_responsemaker *resp = &c->request->response;
    size_t written;

    written = writeA(c->fd, (void *)resp->header, resp->headerlen);
    if (written != resp->headerlen) {
        // TODO: write the rest of the buffer later after pcaio_relaxA
        return -1;
    }

    return written;
}


ssize_t
chttpd_response_content_flushA(struct chttpd_connection *c) {
    struct chttp_responsemaker *resp = &c->request->response;
    size_t written;

    written = writeA(c->fd, resp->content, resp->contentlength);
    if (written != resp->contentlength) {
        // TODO: write the rest of the buffer later after pcaio_relaxA
        return -1;
    }

    resp->contentlength = 0;
    return written;
}


ssize_t
chttpd_response_chunk_flushA(struct chttpd_connection *c) {
    struct chttp_responsemaker *resp = &c->request->response;
    struct iovec v[3];
    char head[32];
    int headlen;
    size_t totallen;
    size_t written;

    headlen = sprintf(head, "%X\r\n", (unsigned int)resp->contentlength);
    v[0].iov_base = head;
    v[0].iov_len = headlen;
    v[1].iov_base = resp->content;
    v[1].iov_len = resp->contentlength;
    v[2].iov_base = "\r\n";
    v[2].iov_len = 2;

    totallen = headlen + resp->contentlength + 2;
    written = writevA(c->fd, v, 3);
    if (written != totallen) {
        // TODO: write the rest of the buffer later after pcaio_relaxA
        return -1;
    }

    resp->contentlength = 0;
    return totallen;
}


int
chttpd_response_writechunkA(struct chttpd_connection *c, const char *budd,
        size_t len) {
    // TODO: implement
    return -1;
}


int
chttpd_response_chunk_end(struct chttpd_connection *c) {
    if (writeA(c->fd, "0\r\n\r\n", 5) != 5) {
        return -1;
    }

    return 0;
}
