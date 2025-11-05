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
#include <stdio.h>
#include <arpa/inet.h>

/* local public */
#include "carrot/addr.h"


#define _BUFFLEN 128
static char _out[_BUFFLEN];


const char *
ipaddr2a(const struct ipaddr *addr) {
    char tmp[_BUFFLEN];

    if (addr->family == AF_INET) {
        sprintf(_out, "%s",
                inet_ntop(AF_INET, &addr->v4.s_addr, tmp, _BUFFLEN));
    }
    else if (addr->family == AF_INET6) {
        sprintf(_out, "%s",
                inet_ntop(AF_INET6, &addr->v6.s6_addr, tmp, _BUFFLEN));
    }
    else {
        return NULL;
    }

    return _out;
}


const char *
saddr2a(const union saddr *addr) {
    char tmp[_BUFFLEN];

    if (addr->ss.ss_family == AF_UNIX) {
        sprintf(_out, "%s", addr->sun.sun_path);
    }
    else if (addr->ss.ss_family == AF_INET) {
        sprintf(_out, "%s:%d",
                inet_ntop(AF_INET, &addr->sin.sin_addr, tmp, _BUFFLEN),
                ntohs(addr->sin.sin_port));
    }
    else if (addr->ss.ss_family == AF_INET6) {
        sprintf(_out, "[%s]:%d",
                inet_ntop(AF_INET6, &addr->sin6.sin6_addr, tmp, _BUFFLEN),
                ntohs(addr->sin6.sin6_port));
    }
    else {
        return NULL;
    }

    return _out;
}


const char *
cidr2a(const struct cidr *cidr) {
    char tmp[_BUFFLEN];

    if (cidr->addr.family == AF_INET) {
        sprintf(_out, "%s/%d",
                inet_ntop(AF_INET, &cidr->addr.v4.s_addr, tmp, _BUFFLEN),
                cidr->bits);
    }
    else if (cidr->addr.family == AF_INET6) {
        sprintf(_out, "%s/%d",
                inet_ntop(AF_INET6, &cidr->addr.v6.s6_addr, tmp, _BUFFLEN),
                cidr->bits);
    }
    else {
        return NULL;
    }

    return _out;
}


void
subnetmask(unsigned char bits, struct ipaddr *out) {
    out->family = AF_INET;
    out->v4.s_addr = htonl(~((1 << (32 - bits)) - 1));
}
