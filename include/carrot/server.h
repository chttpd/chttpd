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
#ifndef INCLUDE_CARROT_CARROT_H_
#define INCLUDE_CARROT_CARROT_H_


/* thirdparty */
#include <mrb.h>
#include <chttp/chttp.h>

/* local public */
#include "carrot/addr.h"


typedef struct carrot_server *carrot_server_t;
struct carrot_server_conn;
typedef int (*carrot_handler_t)(struct carrot_server_conn *c, void *ptr);
struct carrot_server_config {
    const char *bind;
    unsigned int backlog;
    unsigned int requestbuffer_mempages;
    unsigned int connectionbuffer_mempages;

    // TODO: apply
    unsigned int connections_max;
};


extern const struct carrot_server_config carrot_server_defaultconfig;


struct carrot_server_conn {
    int fd;
    union saddr peer;
    struct mrb ring;
    struct chttp_request *request;
};


void
carrot_server_makedefaults(struct carrot_server_config *c);


struct carrot_server *
carrot_server_new(const struct carrot_server_config *c);


void
carrot_server_free(struct carrot_server *s);


int
carrot_server_route(struct carrot_server *s, const char *verb, const char *path,
        carrot_handler_t handler, void *ptr);


int
carrot_server_main(struct carrot_server *s);


int
carrot_serverA(int argc, void *argv[]);


int
carrot_server_connA(int argc, void *argv[]);


ssize_t
carrot_server_responseA(struct carrot_server_conn *c, int status,
        const char *text, const char *content, size_t contentlen);


ssize_t
carrot_server_rejectA(struct carrot_server_conn *c, int status,
        const char *text);


ssize_t
carrot_server_recvchunkA(struct carrot_server_conn *c, const char **start);


ssize_t
carrot_server_recvsearchA(struct carrot_server_conn *c, const char *s);


int
carrot_server_recvallA(struct carrot_server_conn *c, char **out);


ssize_t
carrot_server_sendpacketA(struct carrot_server_conn *c,
        struct chttp_packet *p);


#endif  // INCLUDE_CARROT_CARROT_H_
