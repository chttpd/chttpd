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
#include <cutest.h>


int
headparse(char *headers);


int
reqparse(char **verb, char **path, char **version, char **headers,
        char **body);


void
test_request_parser() {
    const char* request_str = "POST /api/users HTTP/1.1\r\n"
                              "Host: example.com\r\n"
                              "Content-Type: application/json\r\n"
                              "\r\n"
                              "{\"foo\":\"bar\",\"baz\":30}";

    char *verb;
    char *path;
    char *version;
    char *body;

    eqint(0, reqparse(&verb, &path, &version, &headers, &body));
    eqstr(verb, "POST");
    eqstr(path, "/api/users");
    eqstr(version, "HTTP/1.1");
    // eqstr(headers, ...);
    eqstr(body, "{\"foo\":\"bar\",\"baz\":30}");
}


int
main() {
    test_request_parser();
    return EXIST_SUCCESS;
}
