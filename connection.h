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
#ifndef CONNECTION_H_
#define CONNECTION_H_


#include "chttpd.h"


/**
 * @brief Creates a new connection.
 *
 * This function creates a new chttpd_connection struct and initializes its
 * fields with the provided parameters. Memory is allocated for the struct
 * using malloc.
 * Also the New chttpd_connection is initialized with zero to ensure
 * providing a consistent and known initial state for the chttpd_connection.
 *
 * @param chttpd The chttpd struct that the connection belongs to.
 * @param fd The file descriptor associated with the connection.
 * @param addr The remote address of the connection.
 * @return A pointer to the newly created chttpd_connection struct, or NULL if
 * allocation fails.
 */
struct chttpd_connection*
chttpd_connection_new(struct chttpd *chttpd, int fd, struct sockaddr addr);


/**
 * @brief Disposes chttpd_connection struct and releases associated resources.
 *
 * This function disposes a chttpd_connection struct by releasing the
 * resources associated with it. It unregisters the file descriptor from the
 * event loop, closes the file descriptor, destroys the input and output
 * buffers, and frees the memory allocated for the chttpd_connection struct.
 *
 * @param req Pointer to the chttpd_connection struct to dispose of.
 */
void
chttpd_connection_dispose(struct chttpd_connection *req);


#endif  // CONNECTION_H_
