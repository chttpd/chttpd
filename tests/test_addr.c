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


// TODO: test addr.c

// static void
// test_saddr_slit() {
//     // eqint(saddr_split(
// }


static void
test_saddr2a() {
    char buff[32];
    union saddr saddr;

    saddr.sin_port = htons(8080);

    /* inet_pton returns 1 on success */
    eqint(1, inet_pton(AF_INET, "127.0.0.1", &saddr.sin_addr));
    saddr.sa_family = AF_INET;
    eqint(0, saddr2a(buff, sizeof(buff), &saddr));
    eqstr("127.0.0.1:8080", buff);
}


int
main() {
    test_saddr2a();
    return EXIT_SUCCESS;
}
