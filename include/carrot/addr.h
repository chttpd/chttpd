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
    sa_family_t family;
    union {
        struct in_addr;
        struct in6_addr;
    };
};


union saddr {
    struct sockaddr_storage;
    struct sockaddr;
    struct sockaddr_un;
    struct sockaddr_in;
    struct sockaddr_in6;
};


struct cidr {
    struct ipaddr addr;
    uint8_t bits;
};


int
ipaddr_fromstr(struct ipaddr *dst, const char *src);


int
in6addr_fromstr(struct in6_addr *dst, const char *src);


int
ipaddr_tostr(char *dst, size_t dstlen, const struct ipaddr *addr);


void
ipaddr_netmask(struct ipaddr *out, unsigned char bits);


int
saddr_fromstr(union saddr *dst, const char *src);


int
saddr_tostr(char *dst, size_t dstlen, const union saddr *addr);


int
saddr_split(const char **node, const char **service, char *in);


/** freeaddrinfo must be called later to freeup the result
 */
int
saddr_resolveA(struct addrinfo **result, const char *src);


#endif  // INCLUDE_CARROT_ADDR_H_
