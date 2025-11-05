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
#ifndef INCLUDE_CARROT_ADDR_H_
#define INCLUDE_CARROT_ADDR_H_

/* system */
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>

/* posix */
#include <netdb.h>
#include <netinet/in.h>


struct ipaddr {
    /* AF_INET or AF_INET6 */
    int family;
    union {
        struct in_addr v4;
        struct in6_addr v6;
    };
};


union saddr {
    struct sockaddr_storage ss;
    struct sockaddr sa;
    struct sockaddr_un sun;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
};


struct cidr {
    struct ipaddr addr;
    uint8_t bits;
};


const char *
ipaddr2a(const struct ipaddr *addr);


const char *
saddr2a(const union saddr *addr);


const char *
cidr2a(const struct cidr *cidr);


void
subnetmask(unsigned char bits, struct ipaddr *out);


#endif  // INCLUDE_CARROT_ADDR_H_
