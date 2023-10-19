// Copyright 2023 Vahid Mardani
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
#include <stdlib.h>

#include <chttpd.h>


#define RESP_HEADER "<!doctype html><html><head>" \
    "<title>chttpd demo</title>" \
    "</head><body>"
#define RESP_FOOTER "</body></html>"


static int
newconnection(struct chttpd *chttpd, int fd, struct sockaddr addr) {
    INFO("new connection: %s", sockaddr_dump(&addr));
    return 0;
}


static int
closeconnection(struct chttpd *chttpd, int fd, struct sockaddr addr) {
    INFO("close connection: %s", sockaddr_dump(&addr));
    return 0;
}


static int
newrequest(struct chttpd_connection *req) {
    INFO("new request: %s %s", req->verb, req->path);
    return 0;
}


static int
endrequest(struct chttpd_connection *req) {
    INFO("end request: %s %s", req->verb, req->path);
    return 0;
}


static ASYNC
indexA(struct caio_task *self, struct chttpd_connection *req) {
    CORO_START;
    chttpd_response(req, "200 OK", "text/html", RESP_HEADER
            "<h1>Hello %s!</h1>" RESP_FOOTER, "chttpd");
    CORO_FINALLY;
}


static struct chttpd_route routes[] = {
    CHTTPD_ROUTE("^/$", NULL, indexA),
    CHTTPD_ROUTE_TERMINATOR
};


int
main() {
    struct chttpd chttpd;

    clog_verbosity = CLOG_DEBUG;
    chttpd_defaults(&chttpd);
    chttpd.backlog = 1000;
    chttpd.maxconn = 1000;
    chttpd.routes = routes;
    chttpd.on_connection_open = newconnection;
    chttpd.on_connection_close = closeconnection;
    chttpd.on_request_begin = newrequest;
    chttpd.on_request_end = endrequest;
    return chttpd_forever(&chttpd);
}
