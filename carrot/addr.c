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


// const char *
// ipaddr2a(const struct ipaddr *addr) {
//     char tmp[_BUFFLEN];
//
//     if (addr->family == AF_INET) {
//         sprintf(_out, "%s",
//                 inet_ntop(AF_INET, &addr->v4.s_addr, tmp, _BUFFLEN));
//     }
//     else if (addr->family == AF_INET6) {
//         sprintf(_out, "%s",
//                 inet_ntop(AF_INET6, &addr->v6.s6_addr, tmp, _BUFFLEN));
//     }
//     else {
//         return NULL;
//     }
//
//     return _out;
// }


int
saddr2a(char *dst, size_t dstlen, const union saddr *saddr) {
    int ret;
    char tmp[32];

    if (saddr->ss_family == AF_UNIX) {
        ret = snprintf(dst, dstlen, "%s", saddr->sun_path);
    }
    else if (saddr->ss_family == AF_INET) {
        ret = snprintf(dst, dstlen, "%s:%d",
                inet_ntop(AF_INET, &saddr->sin_addr, tmp, sizeof(tmp)),
                ntohs(saddr->sin_port));
    }
    else if (saddr->ss_family == AF_INET6) {
        ret = snprintf(dst, dstlen, "[%s]:%d",
                inet_ntop(AF_INET6, &saddr->sin6_addr, tmp, sizeof(tmp)),
                ntohs(saddr->sin6_port));
    }
    else {
        return -1;
    }

    if (ret >= dstlen) {
        return -1;
    }

    return 0;
}


// const char *
// cidr2a(const struct cidr *cidr) {
//     char tmp[_BUFFLEN];
//
//     if (cidr->addr.family == AF_INET) {
//         sprintf(_out, "%s/%d",
//                 inet_ntop(AF_INET, &cidr->addr.v4.s_addr, tmp, _BUFFLEN),
//                 cidr->bits);
//     }
//     else if (cidr->addr.family == AF_INET6) {
//         sprintf(_out, "%s/%d",
//                 inet_ntop(AF_INET6, &cidr->addr.v6.s6_addr, tmp, _BUFFLEN),
//                 cidr->bits);
//     }
//     else {
//         return NULL;
//     }
//
//     return _out;
// }


void
subnetmask(unsigned char bits, struct ipaddr *dst) {
    dst->family = AF_INET;
    dst->v4.s_addr = htonl(~((1 << (32 - bits)) - 1));
}


int
ipaddr_fromstr(struct ipaddr *dst, sa_family_t fam, const char *src) {
    const char *dot;

    dot = strchr(src, '.');
    if (dot) {
        /* v4 */

    }
    else {
        /* v6 */
    }

    return -1;
}
