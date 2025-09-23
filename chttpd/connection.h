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
#ifndef CHTTPD_CONNECTION_H_
#define CHTTPD_CONNECTION_H_


/* thirdparty */
#include <mrb.h>
#include <chttp.h>

/* local public */
#include "chttpd/addr.h"


struct connection {
    /* this member should be always the first */
    struct chttp_request req;

    int fd;
    union saddr peeraddr;

    mrb_t ring;
};


int
connectionA(int argc, void *argv[]);


/** search inside the input ring buffer.
 * returns:
 * -1: not found
 * -2: buffer is full and not found.
 *  n: length of found string.
 */
int
connection_ring_search(struct connection *c, const char *s);


#endif  // CHTTPD_CONNECTION_H_
