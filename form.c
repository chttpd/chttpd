#include <stdlib.h>
// #include <caio.h>
//
#include "chttpd.h"


int
chttpd_formparser_new(struct chttpd_connection *req) {
    struct chttpd_formparser *parser = malloc(sizeof(struct chttpd_formparser));
    if (parser == NULL) {
        return -1;
    }

    return 0;
}


void
chttpd_formparser_dispose(struct chttpd_connection *req) {
    if (req->formparser == NULL) {
        return;
    }

    free(req->formparser);
}


ASYNC
chttpd_formfield_next(struct caio_task *self,
        struct chttpd_formparser *parser) {
    CORO_START;

    CORO_YIELD(NULL);
    CORO_FINALLY;
}
