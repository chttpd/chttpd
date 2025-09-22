// Copyright 2025 Vahid Mardani
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
/* standard */
#include <errno.h>

/* system */
#include <sys/types.h>
#include <sys/socket.h>

/* posix */
#include <netdb.h>

/* thirdparty */
#include <clog.h>

/* local private */
#include "socket.h"

/* local public */
#include "chttpd/addr.h"


static int
_unix_bind(const char *addr, union saddr *out) {
    int fd;
    struct sockaddr_un name;
    size_t addrlen;

    if (addr == NULL) {
        return -1;
    }

    addrlen = strlen(addr);
    if ((addrlen == 0) || (addrlen >= 108)) {
        return -1;
    }

    fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd == -1) {
        return -1;
    }

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, addr, addrlen);

    /* bind */
    if (bind(fd, (const struct sockaddr *) &name, sizeof(name)) == -1) {
        close(fd);
        ERROR("bind(fd: %d, path: %s)", fd, addr);
        return -1;
    }

    if (out) {
        memcpy(out, &name, sizeof(name));
    }
    return fd;
}


static int
_inet_bind(const char *addr, const char *service, union saddr *out) {
    int fd = -1;
    int reuse = 1;
    int err;
    struct addrinfo *result;
    struct addrinfo hint;
    struct addrinfo *cur;

    memset(&hint, 0, sizeof(hint));
    /* allow ipv4 and ipv6 */
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = 0;
    hint.ai_protocol = 0;

    err = getaddrinfo(addr, service, &hint, &result);
    if (err) {
        ERROR("getaddrinfo: %s", gai_strerror(err));
        return -1;
    }

    /* getaddrinfo() returns a list of address structures. Try each address
     * until we successfully connect(2). If socket(2) (or connect(2)) fails,
     * we (close the socket and) try the next address.
     */
    for (cur = result; cur != NULL; cur = cur->ai_next) {
        errno = 0;
        fd = socket(cur->ai_family, cur->ai_socktype | SOCK_NONBLOCK,
                cur->ai_protocol);
        if (fd == -1) {
            continue;
        }

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                (void *)&reuse, sizeof(reuse)) == 0) {
            if (bind(fd, cur->ai_addr, cur->ai_addrlen) == 0) {
                /* success */
                break;
            }
        }

        close(fd);
        fd = -1;
    }

    if (fd == -1) {
        ERROR("bind(%s:%s)", addr, service);
    }
    else if (out) {
        memcpy(out, cur->ai_addr, cur->ai_addrlen);
    }

    freeaddrinfo(result);
    return fd;
}


int
socket_bind(const char *addr, union saddr *out) {
    int ret;
    char *tmp;
    char *service;

    if (strncmp(addr, "unix://", 7) == 0) {
        return _unix_bind(addr + 7, out);
    }

    tmp = strdup(addr);
    service = strchr(tmp, ':');
    if (service == NULL) {
        return -1;
    }
    service[0] = 0;
    service++;

    if (service[0] == 0) {
        return -1;
    }

    ret = _inet_bind(tmp, service, out);
    free(tmp);
    return ret;
}
