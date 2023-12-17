#include "chttpd.h"
#include "helpers.h"


ASYNC
body_readA(struct caio_task *self, struct chttpd_connection *req) {
    int bytes;
    int toread;
    CORO_START;

    if (req->contentlength == -1) {
        /* Chunked transfer encoding is not soppurted yet */
        CORO_REJECT(CHTTPD_ENO_CONTENTLENGTHMISSING);
    }

    if (req->contentlength == 0) {
        /* Nothiing to read */
        CORO_RETURN;
    }

    toread = mrb_available(req->inbuff);
    if (toread < req->contentlength) {
        CORO_REJECT(CHTTPD_ENO_REQUESTTOOLONG);
    }

    req->remainingbytes = req->contentlength;
    while (req->remainingbytes) {
        errno = 0;
        toread = MIN(mrb_available(req->inbuff), req->remainingbytes);
        bytes = mrb_readin(req->inbuff, req->fd, toread);
        if (bytes == 0) {
            CORO_REJECT(CHTTPD_ENO_CONNECTIONCLOSED);
        }

        if (bytes > 0) {
            req->remainingbytes -= bytes;
            continue;
        }

        if (CORO_MUSTWAITFD(req->fd)) {
            CORO_WAITFD(req->fd, EIN);
        }
        else {
            // TODO: timeout error
            CORO_REJECT(bytes);
        }
    }

    CORO_FINALLY;
}
