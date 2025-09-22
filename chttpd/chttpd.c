/* local public */
#include "chttpd/chttpd.h"


void
chttpd_config_default(struct chttpd_config *c) {
    c->bind = "127.0.0.1:80";
    c->backlog = 10;
}
