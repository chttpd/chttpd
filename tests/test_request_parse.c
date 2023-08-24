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
#include <clog.h>


struct chttpd_request {
    char *verb;
    char *path;
    char *version;
    char *content_length;
    char *content_type;
    char *connection;
    char *accept;
    char *httpheader;
};


void
trim(char **str) {
    while ((**str == ' ') || (**str == '\t')) {
        (*str)++;
    }
}


int
reqtok(char *request, char **verb, char **path, char **version, char **accept,
        char **connection, char **content_type, char **content_length) {
    
    char *saveptr;
    char *saveptr_line;
    char *token;

    char *line = strtok_r(request, "\r\n", &saveptr_line);
    
    *verb = strtok_r(line, " ", &saveptr);
    *path = strtok_r(NULL, " ", &saveptr);
    *version = strtok_r(NULL, "HTTP/", &saveptr);

    while ((line = strtok_r(NULL, "\r\n", &saveptr_line))) {
        token = strtok_r(line, ":", &saveptr);

        if (strcasecmp(token, "Accept") == 0) {
            *accept = strtok_r(NULL, "", &saveptr);
            trim(accept);
        }
        else if (strcasecmp(token, "Connection") == 0) {
            *connection = strtok_r(NULL, "", &saveptr);
            trim(connection);
        }
        else if (strcasecmp(token, "Content-Type") == 0) {
            *content_type = strtok_r(NULL, "", &saveptr);
            trim(content_type);
        }
        else if (strcasecmp(token, "Content-Length") == 0) {
            *content_length = strtok_r(NULL, "", &saveptr);
            trim(content_length);
        }
    }

    return 0;
}


int
request_parse(char *request, struct chttpd_request *conn) {
    int rs;
    
    conn->httpheader = request;
    rs = reqtok(request, &conn->verb, &conn->path, &conn->version,
            &conn->accept, &conn->connection, &conn->content_type,
            &conn->content_length);
    
    if (rs != 0) {
        return -1;   
    }
    
    return 0;
}


void
test_request_parse() {
    char request[] = "GET /foo/bar HTTP/1.1\r\n"
            "Accept: baz\r\nConnection: corge\r\nContent-Type: qux/quux\r\n"
            "Host: foohost\r\n\r\n";

    struct chttpd_request conn;

    eqint(0, request_parse(request, &conn));
    eqstr("GET", conn.verb);
    eqstr("/foo/bar", conn.path);
    eqstr("1.1", conn.version);
    eqstr(conn.accept, "baz");
    eqstr(conn.connection, "corge");
    eqstr(conn.content_type, "qux/quux");
    isnull(conn.content_length);
    INFO(conn.httpheader);
    eqstr(conn.httpheader, "Accept: baz\r\n"
            "Connection: corge\r\nContent-Type: qux/quux\r\nHost: foohost");
}

// TODO:
// 1. Fix httpheader and httpheader_count 

void
main() {
    test_request_parse();
}
