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
#ifndef TESTS_TESTHELPERS_H_
#define TESTS_TESTHELPERS_H_


#include <stdio.h>
#include <unistd.h>


struct tfile {
    FILE *file;
    int fd;
};


static struct tfile
tmpfile_open() {
    struct tfile t = {
        .file = tmpfile(),
    };

    t.fd = fileno(t.file);
    return t;
}


#define REQ(r) \
    in = (r); \
    lseek(req.fd, 0, SEEK_SET); \
    ftruncate(req.fd, 0); \
    write(req.fd, in, strlen(in)); \
    lseek(req.fd, 0, SEEK_SET)


#endif  // TESTS_TESTHELPERS_H_
