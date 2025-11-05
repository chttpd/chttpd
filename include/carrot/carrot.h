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


struct carrot_connection;
typedef int (*carrot_handler_t)(struct carrot_connection *c, void *ptr);
typedef struct carrot *carrot_t;
struct carrot_config {
    const char *bind;
    unsigned int backlog;
    unsigned int requestbuffer_mempages;
    unsigned int connectionbuffer_mempages;

    // TODO: apply
    unsigned int connections_max;
};


extern const struct carrot_config carrot_defaultconfig;


struct carrot_connection {
    int fd;
    union saddr peer;
    struct mrb ring;
    struct chttp_request *request;
};


void
carrot_config_makedefaults(struct carrot_config *c);


struct carrot *
carrot_new(const struct carrot_config *c);


void
carrot_free(struct carrot *s);


int
carrot_route(struct carrot *s, const char *verb, const char *path,
        carrot_handler_t handler, void *ptr);


int
carrot_main(struct carrot *s);


int
carrotA(int argc, void *argv[]);


ssize_t
carrot_responseA(struct carrot_connection *c, int status, const char *text,
        const char *content, size_t contentlen);


ssize_t
carrot_response_errorA(struct carrot_connection *c, int status,
        const char *text);


// ssize_t
// carrot_response_flushA(struct carrot_connection *c);
//
//
// int
// carrot_response_contenttype(struct carrot_connection *c, const char *type,
//         const char *charset);
//
//
// int
// carrot_response_header(struct carrot_connection *c, const char *fmt, ...);
//
//
// ssize_t
// carrot_response_write(struct carrot_connection *c, const char *fmt, ...);
//
//
// int
// carrot_response_header_close(struct carrot_connection *c);


// ssize_t
// carrot_response_header_flushA(struct carrot_connection *c);
//
//
// ssize_t
// carrot_response_content_flushA(struct carrot_connection *c);
//
//
// ssize_t
// carrot_response_flushchunkA(struct carrot_connection *c);
//
//
// ssize_t
// carrot_response_writechunkA(struct carrot_connection *c, const char *budd,
//         size_t len);
//
//
// int
// carrot_response_chunk_end(struct carrot_connection *c);


ssize_t
carrot_connection_search(struct carrot_connection *c, const char *s);


ssize_t
carrot_request_readchunkA(struct carrot_connection *c, const char **start);


ssize_t
carrot_connection_readsearchA(struct carrot_connection *c, const char *s);


int
carrot_connection_atleastA(struct carrot_connection *c, size_t count);


int
carrot_connection_readallA(struct carrot_connection *c, char **out);


ssize_t
carrot_connection_sendpacket(struct carrot_connection *c,
        struct chttp_packet *p);


#endif  // INCLUDE_CARROT_CARROT_H_
