#include "chttpd.h"
#include "helpers.h"


ASYNC
body_readA(struct caio_task *self, struct chttpd_connection *req) {
    int bytes;
    int toread;
    CAIO_BEGIN(self);

    if (req->contentlength == -1) {
        /* Chunked transfer encoding is not soppurted yet */
        CAIO_THROW(self, CHTTPD_ENO_CONTENTLENGTHMISSING);
    }

    if (req->contentlength == 0) {
        /* Nothiing to read */
        CAIO_RETURN(self);
    }

    used = mrb_used(req->inbuff);
    toread = mrb_available(req->inbuff);
    if (toread < req->contentlength) {
        CAIO_THROW(self, CHTTPD_ENO_REQUESTTOOLONG);
    }

    req->remainingbytes = req->contentlength;
    while (req->remainingbytes) {
        errno = 0;
        toread = MIN(mrb_available(req->inbuff), req->remainingbytes);
        DEBUG("Reading %d", toread);
        bytes = mrb_readin(req->inbuff, req->fd, toread);
        DEBUG("read %d", bytes);
        if (bytes == 0) {
            CAIO_THROW(self, CHTTPD_ENO_CONNECTIONCLOSED);
        }

        if (bytes > 0) {
            req->remainingbytes -= bytes;
            continue;
        }

        if (CAIO_MUSTWAITFD()) {
            CAIO_WAITFD(self, req->fd, CAIO_IN);
        }
        else {
            // TODO: timeout error
            CAIO_THROW(self, bytes);
        }
    }

    CAIO_FINALLY(self);
}
