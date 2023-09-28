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

/*

#include <sys/socket.h>

struct sockaddr {
    sa_family_t     sa_family;      // Address family
    char            sa_data[];      // Socket address
};

struct sockaddr_storage {
    sa_family_t     ss_family;      // Address family
};

typedef socklen_t;
typedef sa_family_t;

Internet domain sockets
#include <netinet/in.h>

struct sockaddr_in {
    sa_family_t     sin_family;     // AF_INET
    in_port_t       sin_port;       // Port number
    struct in_addr  sin_addr;       // IPv4 address
};

struct sockaddr_in6 {
    sa_family_t     sin6_family;    // AF_INET6
    in_port_t       sin6_port;      // Port number
    uint32_t        sin6_flowinfo;  // IPv6 flow info
    struct in6_addr sin6_addr;      // IPv6 address
    uint32_t        sin6_scope_id;  // Set of interfaces for a scope
};

struct in_addr {
    in_addr_t s_addr;
};

struct in6_addr {
    uint8_t   s6_addr[16];
};

typedef uint32_t in_addr_t;
typedef uint16_t in_port_t;

UNIX domain sockets
#include <sys/un.h>

struct sockaddr_un {
    sa_family_t     sun_family;     // Address family
    char            sun_path[];     // Socket pathname
};
*/
#include <stdio.h>
#include <string.h>
#include <sys/un.h>

#include "chttpd.h"
#include "networking.h"


// TODO: Tune it
#define TMPMAX 256
static char addrtemp[TMPMAX];


char *
sockaddr_dump(struct sockaddr *addr) {
    struct sockaddr_un *addrun;
    struct sockaddr_in *addrin;

    if (addr->sa_family == AF_UNIX) {
        addrun = (struct sockaddr_un*) addr;
        snprintf(addrtemp, TMPMAX, "%s", addrun->sun_path);
    }
    else {
        addrin = (struct sockaddr_in*) addr;
        snprintf(addrtemp, TMPMAX, "%s:%d", inet_ntoa(addrin->sin_addr),
            ntohs(addrin->sin_port));
    }
    return addrtemp;
}


int
sockaddr_parse(struct sockaddr *restrict saddr, const char *addr,
        unsigned short port) {
    memset(saddr, 0, sizeof(struct sockaddr_storage));
    int addrlen = strlen(addr);

    /* Unix domain socket */
    if ((addrlen > 7) && (strncmp("unix://", addr, 7) == 0)) {
        struct sockaddr_un *addrun = (struct sockaddr_un*)saddr;
        addrlen -= 7;
        if (addrlen > 107) {
            ERROR("Unix domain socket path is greater than 107");
            return -1;
        }
        addrun->sun_family = AF_UNIX;
        strncpy(addrun->sun_path, addr + 7, addrlen);
        return sizeof(struct sockaddr_un);
    }

    /* inet socket */
    struct sockaddr_in *addrin = (struct sockaddr_in*)saddr;
    addrin->sin_family = AF_INET;
    if (addr == NULL) {
        addrin->sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else if (inet_pton(AF_INET, addr, &addrin->sin_addr) <= 0) {
        return -1;
    }
    addrin->sin_port = htons(port);
    return sizeof(struct sockaddr_in);
}


int
chttpd_listen(struct chttpd *chttpd) {
    int addrlen;
    int option = 1;
    int fd;
    struct sockaddr *saddr = (struct sockaddr*)&chttpd->listenaddr;

    /* Parse the bind address */
    addrlen = sockaddr_parse(saddr, chttpd->bindaddr, chttpd->bindport);
    if (addrlen == -1) {
        ERROR("Invalid address: %s:%d", chttpd->bindaddr, chttpd->bindport);
        return -1;
    }
    chttpd->listenaddrlen = (socklen_t)addrlen;
    ERROR("Listenning on: %s", sockaddr_dump(saddr));
    return -1;

    /* Create socket */
    fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd == -1) {
        ERROR("Cannot create socket: %s:%s", chttpd->bindaddr,
                chttpd->bindport);
        return -1;
    }

    /* Allow reuse the address */
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option))) {
        ERROR("Cannot set socket option: %s:%s", chttpd->bindaddr,
                chttpd->bindport);
        return -1;
    }

    /* Bind to socket */
    if (bind(fd, saddr, chttpd->listenaddrlen)) {
        ERROR("Cannot bind on: %s", sockaddr_dump(saddr));
        return -1;
    }

    /* Listen */
    if (listen(fd, chttpd->backlog)) {
        ERROR("Cannot listen on: %s", sockaddr_dump(saddr));
        return -1;
    }

    INFO("Listening on: %s", sockaddr_dump(saddr));
    return fd;
}
