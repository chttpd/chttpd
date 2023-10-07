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
#ifndef ROUTER_H_
#define ROUTER_H_


#include "chttpd.h"


/**
 * @brief Routes a request to the appropriate handler based on the path.
 *
 * It iterates over the route array in the chttpd instance and matches
 * the request path against the regular expressions defined for each route.
 * If a match is found, the corresponding handler is set in the request
 * struct, and the function returns the index of the matched route. If no
 * match is found, -1 is returned.
 *
 * @param req Pointer to the chttpd_connection struct representing the request.
 * @return The index of the matched route on success, or -1 if no match is found.
 */
int
chttpd_route(struct chttpd_connection *req);


/**
 * @brief Compiles route patterns for chttpd_router.
 *
 * This function compiles the route patterns specified in the chttpd_route
 * struct. Each pattern is compiled using the POSIX regular expression
 * library (regcomp). The compiled regular expressions are stored in the
 * preg field of each route.
 * https://developer-old.gnome.org/glib/stable/glib-regex-syntax.html
 *
 * @param route The chttpd_route struct containing the route patterns to
 * be compiled.
 * @return 0 on success, or -1 if an error occurs during pattern compilation.
 */
int
chttpd_router_compilepatterns(struct chttpd_route * restrict route);


/**
 * @brief Cleans up the resources used by a route array.
 *
 * It iterates over the route array and frees the compiled regular
 * expressions associated with each route.
 *
 * @param route Pointer to the route array to be cleaned up.
 */
void
chttpd_router_cleanup(struct chttpd_route * restrict route);


#endif  // ROUTER_H_
