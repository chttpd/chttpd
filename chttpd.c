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
#include <sys/timerfd.h>

#include <microhttpd.h>
#include <clog.h>
#include <carrow.h>


typedef struct carrow_microhttpd {
    struct MHD_Daemon *daemon;
    struct MHD_Connection *connection;
    struct MHD_Response *response;
    int processed;
} carrow_microhttpd;


#undef CARROW_ENTITY
#define CARROW_ENTITY carrow_microhttpd
#include <carrow_generic.h>
#include <carrow_generic.c>


struct Route {
    const char *url;
    const char *method;
    void (*handler)(struct carrow_microhttpd_coro *self,
            struct carrow_microhttpd *state);
};


static int
maketimer(unsigned int interval) {
    int fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
    if (fd == -1) {
        return -1;
    }

    struct timespec sec1 = {interval, 0};
    struct itimerspec spec = {sec1, sec1};
    if (timerfd_settime(fd, 0, &spec, NULL) == -1) {
        return -1;
    }
    return fd;
}


void
handle_goodbye(struct carrow_microhttpd_coro *self,
        struct carrow_microhttpd *state) {
    CORO_START;
    unsigned long tmp;
    ssize_t bytes;

    self->fd = maketimer(1);
    if (self->fd == -1) {
        CORO_REJECT("maketimer");
    }

    CORO_WAIT(self->fd, CIN);
    bytes = read(self->fd, &tmp, sizeof(tmp));
    DEBUG("Read bytes %zd.", bytes);
    const char *msg = "Goodbye, world!";
    state->response = MHD_create_response_from_buffer(
            strlen(msg), (void *) msg, MHD_RESPMEM_PERSISTENT);
    state->processed = 1;

    CORO_FINALLY;
    if (self->fd != -1) {
        carrow_microhttpd_evloop_unregister(self->fd);
        close(self->fd);
    }
    CORO_END;
}


struct Route routing_table[] = {
    {"/goodbye", "GET", handle_goodbye},
    {NULL, NULL, NULL}
};


static enum MHD_Result
request_handler(void *cls, struct MHD_Connection *connection, const char *url,
        const char *method, const char *version, const char *upload_data,
        size_t *upload_data_size, void **req_cls) {
    extern struct Route routing_table[];
    struct MHD_Response *response;
    enum MHD_Result ret;

    // TODO: Use direct access via hashmap instead of looping.
    int i;
    for (i = 0; routing_table[i].url != NULL; i++) {
        if (strcmp(url, routing_table[i].url) == 0 &&
                strcmp(method, routing_table[i].method) == 0) {
            struct carrow_microhttpd handler_state = {
                .connection = connection,
                .response = NULL,
                .processed = 0
            };
            carrow_microhttpd_coro_create_and_run(routing_table[i].handler,
                    &handler_state);

            if (handler_state.processed == 1) {
                if (handler_state.response != NULL) {
                    ret = MHD_queue_response(handler_state.connection,
                            MHD_HTTP_OK, handler_state.response);
                    MHD_destroy_response(handler_state.response);
                    return ret;
                } else {
                    // Error happened.
                }
            } else {
                // Tell MHD to wait
            }
        }
    }

    const char *not_found = "Not found";
    response = MHD_create_response_from_buffer(
            strlen(not_found), (void *) not_found, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
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
main() {
    struct MHD_Daemon *d;
    int port;

    port = 8080;
    clog_verbosity = CLOG_DEBUG;

    d = MHD_start_daemon(MHD_USE_DEBUG, port, NULL, NULL, &request_handler,
            NULL, MHD_OPTION_END);

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
