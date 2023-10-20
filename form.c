// #include <caio.h>
//
#include "chttpd.h"


int
chttpd_formparser_new(struct chttpd_connection *req) {
    return -1;
}


ASYNC
chttpd_formfield_next(struct caio_task *self,
        struct chttpd_formparser *parser) {
    CORO_START;

    CORO_YIELD(NULL);
    CORO_FINALLY;
}
