/* standard */
#include <stddef.h>

/* local public */
#include "chttpd/chttpd.h"


void
chttpd_config_default(struct chttpd_config *c) {
    c->bind = "127.0.0.1:80";
    c->backlog = 10;
}


struct chttpd *
chttpd_new(struct chttpd_config *c) {
    return NULL;
}


int
chttpdA(int argc, void *argv[]) {
    return -1;
}
