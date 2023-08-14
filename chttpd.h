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
#ifndef CHTTPD_H_
#define CHTTPD_H_


typedef struct chttpd {
    const char *bindaddr;
    unsigned short bindport;
    int backlog;
    size_t buffsize;
} chttpd;


#undef CARROW_ENTITY
#define CARROW_ENTITY chttpd
#include <carrow_generic.h>  // NOLINT


void
chttpdA(struct chttpd_coro *self, struct chttpd *state);


#endif  // CHTTPD_H_
