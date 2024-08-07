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
#ifndef HELPERS_H_
#define HELPERS_H_


#include <errno.h>
#include <clog.h>


#define MIN(a, b) ((a) < (b)? (a): (b))


#define ERRNO_SET(eno_) \
    if (errno != 0) { \
        WARN("Overwriting errno from %d to %d", errno, eno_); \
    } \
    errno = eno_


#define STARTSWITH(a, b) (strncasecmp(a, b, strlen(b)) == 0)


#endif  // HELPERS_H_
