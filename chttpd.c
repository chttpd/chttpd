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
#include <getopt.h>
#include <arpa/inet.h>

#include <microhttpd.h>
#include <clog.h>
#include <carrow.h>


typedef struct carrow_microhttpd {
    struct MHD_Daemon *daemon;
} carrow_microhttpd;


#undef CARROW_ENTITY
#define CARROW_ENTITY carrow_microhttpd
#include <carrow_generic.h>
#include <carrow_generic.c>


static enum MHD_Result
request_handler(void *cls, struct MHD_Connection *connection, const char *url,
        const char *method, const char *version, const char *upload_data,
        size_t *upload_data_size, void **req_cls) {
    struct MHD_Response *response;
    enum MHD_Result ret;

    response = MHD_create_response_from_buffer(
        strlen(url),
        (void *) url,
        MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(
        connection,
        MHD_HTTP_OK,
        response);

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
cli_help(void) {
	INFO("Usage: chttpd [OPTIONS]");
	INFO(" -p, --port              Specify the chttpd listening port");
	INFO(" -b, --bind              Specify the chttpd bind address");
	INFO(" -h, --help              Display this help text");
}


static void
httpserverA(struct carrow_microhttpd_coro *self,
        struct carrow_microhttpd *state) {
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


int
main(int argc, char *argv[]) {
    struct MHD_Daemon *d;
    int port = 8080;
    char bind_addr[32] = "127.0.0.1";
    int opt;
    struct sockaddr_in daemon_addr;
    memset(&daemon_addr, 0, sizeof(struct sockaddr_in));

    clog_verbosity = CLOG_DEBUG;

    static struct option options[] = {
        {"help", no_argument, NULL, 'h'},
        {"port", required_argument, 0, 'p'},
        {"bind", required_argument, 0, 'b'},
        {NULL, 0, NULL, 0}
    };

    while ((opt = getopt_long(argc, argv, "hp:b:", options, NULL)) != -1) {
        switch (opt) {
            case 'p':
                parse_port(optarg, &port);
                break;
            case 'b':
                parse_bind_addr(optarg, bind_addr, &port);
                break;
            case 'h':
                cli_help();
                return 1;
            default:
                cli_help();
                return 1;
        }
    }

    daemon_addr.sin_family = AF_INET;
    daemon_addr.sin_port = htons(port);;
    daemon_addr.sin_addr.s_addr = inet_addr(bind_addr);

    d = MHD_start_daemon (MHD_USE_ERROR_LOG, 0, NULL, NULL, &request_handler,
            NULL, MHD_OPTION_SOCK_ADDR, &daemon_addr, MHD_OPTION_END);

    if (d == NULL) {
        ERROR("Could not start daemon on port %d.\n", port);
        return 1;
    }

    INFO("Listening on port %d...\n", port);

    struct carrow_microhttpd state = {
        .daemon = d
    };

    return carrow_microhttpd_forever(httpserverA, &state, NULL);
}
