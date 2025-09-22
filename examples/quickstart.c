/* local public */
#include "chttpd/chttpd.h"


// static int
// _index(struct chttpd_request *req, void *ptr) {
//     httpd_response_start(req, 200, NULL);
//     httpd_response_contenttype_set(req, "text/plain", "utf-8");
//     httpd_response_write(req, "Hello %s", __FILE__);
//     httpd_response_send(req);
//     return 0;
// }


int
main() {
    chttpd_t server;
    struct chttpd_config c;

    chttpd_config_default(&c);
    server = chttpd_new(&c);
    // chttpd_route(server, "/", "GET", _index, NULL);
    return chttpd_main(server);
}
