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
#include <string.h>

#include <microhttpd.h>

#include "handlers.h"


enum MHD_Result
handle_hello(struct MHD_Connection *connection) {
    const char *msg = "Hello, world!";
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(msg), (void *) msg, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}


enum MHD_Result
handle_goodbye(struct MHD_Connection *connection) {
    const char *msg = "Goodbye, world!";
    struct MHD_Response *response = MHD_create_response_from_buffer(
        strlen(msg), (void *) msg, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}


enum MHD_Result
handle_echo(struct MHD_Connection *connection) {
    const char *data = MHD_lookup_connection_value(connection,
            MHD_GET_ARGUMENT_KIND, "data");
    if (data == NULL) {
        data = "No data provided";
    }
    struct MHD_Response *response = MHD_create_response_from_buffer(
            strlen(data), (void *) data, MHD_RESPMEM_PERSISTENT);
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}
