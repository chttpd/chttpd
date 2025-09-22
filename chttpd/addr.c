/* standard */
#include <stdio.h>
#include <arpa/inet.h>

/* local public */
#include "chttpd/addr.h"


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
