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
#include <stdlib.h>
#include <arpa/inet.h>

/* thirdparty */
#include <chttp/str.h>

/* local public */
#include "carrot/addr.h"

/* local private */
#include "common.h"


void
ipaddr_netmask(struct ipaddr *dst, unsigned char bits) {
    // TODO: ipv6
    dst->family = AF_INET;
    dst->s_addr = htonl(~((1 << (32 - bits)) - 1));
}


int
saddr_unixfromstr(union saddr *dst, const char *src) {
    int max = 108;
    int len;
    char tmp[128];
    char *start;
    char *found;

    ASSRT(src);
    len = strlen(src);
    if (len >= sizeof(tmp)) {
        return -1;
    }

    strncpy(tmp, src, len);
    start = chttp_str_trim(tmp, &len);
    found = strstr(start, "unix://");
    if ((found == NULL) || (found != start)) {
        return -1;
    }

    len -= 7;
    found += 7;
    if (len > max) {
        return -1;
    }

    strcpy(dst->sun_path, found);
    dst->sun_family = AF_UNIX;
    return 0;
}


int
saddr_fromstr(union saddr *dst, const char *src) {
    char tmp[64];
    char *colon;
    unsigned short port;

    ASSRT(src);
    strncpy(tmp, src, 64);
    colon = strrchr(tmp, ':');
    if (colon == NULL) {
        return -1;
    }

    port = atoi(colon + 1);
    if (port == 0) {
        return -1;
    }

    port = htons(port);
    colon[0] = 0;
    if (inet_pton(AF_INET, tmp, &dst->sin_addr)) {
        /* v4 */
        dst->sin_family = AF_INET;
        dst->sin_port = port;
    }
    else if (!in6addr_fromstr(&dst->sin6_addr, tmp)) {
        /* v6 */
        dst->sin6_family = AF_INET6;
        dst->sin6_port = port;
    }
    else if (!saddr_unixfromstr(dst, tmp)) {
        /* unix domain socket failed */
        return -1;
    }

    return 0;
}


int
saddr_tostr(char *dst, size_t dstlen, const union saddr *saddr) {
    int ret;
    char tmp[64];

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


int
ipaddr_tostr(char *dst, size_t dstlen, const struct ipaddr *addr) {
    const void *src;

    if (addr->family == AF_INET) {
        src = &addr->s_addr;
    }
    else {
        src = &addr->s6_addr;
    }

    if (inet_ntop(addr->family, src, dst, dstlen) == NULL) {
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


int
ipaddr_fromstr(struct ipaddr *dst, const char *src) {
    if (strchr(src, '.')) {
        /* v4 */
        ASSRT(1 == inet_pton(AF_INET, src, &dst->s_addr));
        dst->family = AF_INET;
        return 0;
    }

    /* v6 */
    ASSRT(1 == inet_pton(AF_INET6, src, &dst->s6_addr));
    dst->family = AF_INET6;
    return 0;
}


int
in6addr_fromstr(struct in6_addr *dst, const char *src) {
    char tmp[64];
    int len = strlen(src);
    char *start;

    ASSRT(src);
    len = strlen(src);
    ASSRT(len < 64);
    strncpy(tmp, src, len);
    start = tmp;
    if (start[0] == '[') {
        ASSRT(start[len - 1] == ']');
        start++;
        start[len - 2] = 0;
    }

    /* v6 */
    ASSRT(1 == inet_pton(AF_INET6, start, &dst->s6_addr));
    return 0;
}
