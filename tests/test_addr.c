// Copyright 2025 Vahid Mardani
/*
 * This file is part of carrot.
 *  carrot is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  carrot is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with carrot. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Author: Vahid Mardani <vahid.mardani@gmail.com>
 */
/* standard */
#include <unistd.h>
#include <arpa/inet.h>

/* thirdparty */
#include <cutest.h>

/* local public */
#include "carrot/addr.h"


static void
test_saddr_slit() {
    const char *node;
    const char *service;
    char in[32] = "localhost:http";

    eqint(0, saddr_split(&node, &service, in));
    eqstr("localhost", node);
    eqstr("http", service);

    eqint(-1, saddr_split(&node, &service, ":bar"));
    eqint(-1, saddr_split(&node, &service, "foo:"));
    eqint(-1, saddr_split(&node, &service, ":"));
    eqint(-1, saddr_split(&node, &service, "foo"));
}


static void
test_ipaddr_fromtostr() {
    char buff[64];
    struct ipaddr addr;

    eqint(0, ipaddr_fromstr(&addr, "0.0.0.73"));
    eqint(AF_INET, addr.family);
    eqint(htonl(73), addr.s_addr);
    eqint(0, ipaddr_tostr(buff, sizeof(buff), &addr));
    eqstr("0.0.0.73", buff);

    eqint(0, ipaddr_fromstr(&addr, "::1"));
    eqint(AF_INET6, addr.family);
    eqint(0, ipaddr_tostr(buff, sizeof(buff), &addr));
    eqint(1, addr.s6_addr[15]);
    eqstr("::1", buff);
}


static void
test_saddr_fromtostr() {
    char buff[64];
    union saddr saddr;

    eqint(0, saddr_fromstr(&saddr, "[fe80::31fb]:53"));
    eqint(0, saddr_tostr(buff, sizeof(buff), &saddr));
    eqint(AF_INET6, saddr.sin6_family);
    eqstr("[fe80::31fb]:53", buff);

    eqint(0, saddr_fromstr(&saddr, "127.0.0.1:8080"));
    eqint(0, saddr_tostr(buff, sizeof(buff), &saddr));
    eqint(AF_INET, saddr.sin_family);
    eqstr("127.0.0.1:8080", buff);
}


int
main() {
    test_saddr_slit();
    test_ipaddr_fromtostr();
    test_saddr_fromtostr();
    return EXIT_SUCCESS;
}
