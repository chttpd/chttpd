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
#ifndef REQUEST_H_
#define REQUEST_H_


#include <sys/socket.h>

#include <mrb.h>


typedef struct request {
    int fd;
    struct sockaddr localaddr;
    struct sockaddr remoteaddr;
    mrb_t buff;
    void *backref;
} request;


#undef CARROW_ENTITY
#define CARROW_ENTITY request
#include <carrow_generic.h>  // NOLINT


void
requestA(struct request_coro *self, struct request *conn);


#endif  // REQUEST_H_
