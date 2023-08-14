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
#include <stdlib.h>
#include <unistd.h>
#include <argp.h>
#include <arpa/inet.h>

#include <microhttpd.h>
#include <clog.h>
#include <carrow.h>

#include "manifest.h"


typedef struct chttpd {
    struct MHD_Daemon *daemon;
} chttpd;


#undef CARROW_ENTITY
#define CARROW_ENTITY chttpd
#include <carrow_generic.h>  // NOLINT
#include <carrow_generic.c>  // NOLINT


typedef struct request {
    struct MHD_Connection *connection;
    enum MHD_Result result;
    void *ptr;
} request;


#undef CARROW_ENTITY
#define CARROW_ENTITY request
#include <carrow_generic.h>  // NOLINT
#include <carrow_generic.c>  // NOLINT


const char *argp_program_version = CHTTPD_VERSION;
const char *argp_program_bug_address = "<vahid.mardani@gmail.com>";
static char doc[] = "Nonblocking IO HTTP server using Carrow.";
static char args_doc[] = "";


static struct argp_option options[] = {
    {"port", 'p', "PORT", 0, "Bind port"},
    {"bind", 'b', "IPADDRESS", 0, "Bind address"},
    {0}
};


struct options {
    char bind_addr[32];
    int port;
};


void
handle_goodbye(struct request_coro *self,
        struct request *state) {
    const char *msg = "Goodbye, world!";
    struct MHD_Response *response;

    CORO_START;

    response = MHD_create_response_from_buffer(strlen(msg), (void *) msg,
            MHD_RESPMEM_PERSISTENT);
    state->result = MHD_queue_response(state->connection, MHD_HTTP_OK,
            response);
    MHD_destroy_response(response);

    CORO_FINALLY;
    CORO_CLEANUP;
}


struct route {
    const char *url;
    const char *method;
    void (*handler)(struct request_coro *self, struct request *state);
};


struct route routing_table[] = {
    {"/goodbye", "GET", handle_goodbye},
    {NULL, NULL, NULL}
};


static enum MHD_Result
request_handler(void *cls, struct MHD_Connection *connection, const char *url,
        const char *method, const char *version, const char *upload_data,
        size_t *upload_data_size, void **req_cls) {
    enum MHD_Result ret;

    // TODO: Use direct access via hashmap instead of looping.
    int i;
    for (i = 0; routing_table[i].url != NULL; i++) {
        if (strcmp(url, routing_table[i].url) == 0 &&
                strcmp(method, routing_table[i].method) == 0) {
            struct request *handler_state = malloc(sizeof(struct request));
            handler_state->connection = connection;
            request_coro_create_and_run(routing_table[i].handler,
                    handler_state);
            free(handler_state);
            return handler_state->result;
        }
    }

    struct MHD_Response *response;
    const char *not_found = "Not found";

    response = MHD_create_response_from_buffer(strlen(not_found),
            (void *)not_found, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);

    return ret;
}


void
parse_port(const char *port_arg, int *port) {
    bool is_valid = true;

    for (int i = 0; i < strlen(port_arg); i++) {
        if (port_arg[i] < '0' || port_arg[i] > '9') {
            is_valid = false;
            break;
        }
    }

    if (is_valid == false) {
        ERROR("Invalid port number: %s\n", port_arg);
        exit(EXIT_FAILURE);
    }

    *port = atoi(port_arg);
}


void
parse_bind_addr(const char *bind_addr_arg, char *bind_addr, int *port) {
    int colon_pos = -1;

    if (strcmp(bind_addr_arg, "localhost") == 0) {
        bind_addr_arg = "127.0.0.1";
    }

    for (int i = 0; i < strlen(bind_addr_arg); i++) {
        if (bind_addr_arg[i] == ':') {
            colon_pos = i;
            break;
        }
    }

    if (colon_pos == -1) {
        strcpy(bind_addr, bind_addr_arg);
    } else {
        strncpy(bind_addr, bind_addr_arg, colon_pos);
        bind_addr[colon_pos] = '\0';

        parse_port(bind_addr_arg + colon_pos + 1, port);
    }
}


static void
httpserverA(struct chttpd_coro *self, struct chttpd *state) {
    fd_set rs;
    fd_set ws;
    fd_set es;
    int i;

    CORO_START;

    while (true) {
        FD_ZERO(&rs);
        FD_ZERO(&ws);
        FD_ZERO(&es);

        if (MHD_get_fdset(state->daemon, &rs, &ws, &es, NULL) != MHD_YES) {
            CORO_REJECT("MHD_get_fdset failed.");
        }

        for (i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &rs)) {
                CORO_WAIT(i, CIN);

                FD_ZERO(&rs);
                FD_SET(i, &rs);

                if (MHD_run(state->daemon) != MHD_YES) {
                    CORO_REJECT("MHD_run failed.");
                }

                break;
            }
        }
    }

    CORO_FINALLY;
    MHD_stop_daemon(state->daemon);
    CORO_END;
}


static error_t
parse_opt(int key, char *arg, struct argp_state *state) {
    struct options *options = state->input;

    switch (key) {
        case 'p':
            parse_port(arg, &options->port);
            break;
        case 'b':
            parse_bind_addr(arg, options->bind_addr, &options->port);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}


static struct argp argp = { options, parse_opt, args_doc, doc };


int
main(int argc, char *argv[]) {
    struct MHD_Daemon *d;
    struct sockaddr_in daemon_addr;
    struct options options;

    /* Default values. */
    options.port = 8080;
    strcpy(options.bind_addr, "127.0.0.1");

    argp_parse(&argp, argc, argv, 0, 0, &options);

    memset(&daemon_addr, 0, sizeof(struct sockaddr_in));

    clog_verbosity = CLOG_DEBUG;

    daemon_addr.sin_family = AF_INET;
    daemon_addr.sin_port = htons(options.port);
    daemon_addr.sin_addr.s_addr = inet_addr(options.bind_addr);

    d = MHD_start_daemon(MHD_USE_ERROR_LOG, 0, NULL, NULL, &request_handler,
            NULL, MHD_OPTION_SOCK_ADDR, &daemon_addr, MHD_OPTION_END);

    if (d == NULL) {
        ERROR("Could not start daemon on port %d.\n", options.port);
        return 1;
    }

    INFO("Listening on port %d...\n", options.port);

    struct chttpd state = {
        .daemon = d
    };

    return chttpd_forever(httpserverA, &state, NULL);
}
