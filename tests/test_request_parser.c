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


#define HEADERLINE_MAXSIZE 128
#define HEADERS_MAXCOUNT 10


struct chttpd_request {
    char *verb;
    char *path;
    char *version;
    char content_length[HEADERLINE_MAXSIZE];
    char content_type[HEADERLINE_MAXSIZE];
    char connection[HEADERLINE_MAXSIZE];
    char accept[HEADERLINE_MAXSIZE];
    char headers[HEADERLINE_MAXSIZE * HEADERS_MAXCOUNT];
};


void
headparser(char *request, char *accept, char *connection,
        char *content_length, char *content_type) {
    char *saveptr = NULL;
    char *line = strtok_r(request, "\r\n", &saveptr);

    while (line) {
        if (strncmp(line, "Accept: ", 8) == 0) {
            strcpy(accept, line + 8);
        }
        else if (strncmp(line, "Connection: ", 12) == 0) {
            strcpy(connection, line + 12);
        }
        else if (strncmp(line, "Content-Type: ", 14) == 0) {
            strcpy(content_type, line + 14);
        }
        else if (strncmp(line, "Content-Length: ", 16) == 0) {
            strcpy(content_length, line + 16);
        }

        line = strtok_r(NULL, "\r\n", &saveptr);
    }
}


void
test_headparser() {
    char headers[] = "Accept: baz\r\nConnection: corge\r\n"
            "Content-Type: qux/quux\r\nHost: grault\r\nContent-Length: 125"
            "\r\n\r\n";

    char accept[100];
    char connection[100];
    char content_type[100];
    char content_length[100];

    headparser(headers, accept, connection, content_length, content_type);

    eqstr("baz", accept);
    eqstr("corge", connection);
    eqstr("qux/quux", content_type);
    eqstr("125", content_length);
}


void
test_headparser_error() {
    /* No error case is considered. */
}


int
reqtok(char *request, char **saveptr, char **verb, char **path,
        char **version) {
    char *token;

    if (*saveptr != NULL) {
        /* **saveptr must be null. */
        return -1;
    }

    if (request == NULL) {
        /* request is not provided to reqtok. */
        return -1;
    }

    /* Parse the HTTP verb. */
    token = strtok_r(request, " ", saveptr);
    if (token != NULL) {
        *verb = token;
    }
    else {
        return -1;
    }

    /* Parse the URL. */
    token = strtok_r(NULL, " ", saveptr);
    if (token != NULL) {
        *path = token;
    }
    else {
        return -1;
    }

    /* Parse request body. */
    token = strtok_r(NULL, "/", saveptr);
    if (token != NULL) {
        token = strtok_r(NULL, "\r\n", saveptr);
        if (token != NULL) {
            *version = token;
        }
        else {
            return -1;
        }
    }
    else {
        return -1;
    }


    return 0;
}


void
test_reqtok() {
    const char *request = "POST /api/users HTTP/1.1\r\n";

    char *copy = malloc(strlen(request) + 1);
    strcpy(copy, request);

    char *saveptr = NULL;
    char *verb;
    char *path;
    char *version;

    eqint(0, reqtok(copy, &saveptr, &verb, &path, &version));
    eqstr(verb, "POST");
    eqstr(path, "/api/users");
    eqstr(version, "1.1");
}


void
test_reqtok_error() {
    char request[] = "POST /api/users HTTP//1.1\r\n";
    char *saveptr;
    char *verb;
    char *path;
    char *version;

    eqint(-1, reqtok(request, &saveptr, &verb, &path, &version));
}


int
getfirstline(char *header, char *firstline, size_t maxlen) {
    const char *newline = strchr(header, '\n');

    if (newline != NULL) {
        size_t copy_chars = newline - header;

        if (copy_chars > maxlen - 1) {
            copy_chars = maxlen - 1;
        }

        strncpy(firstline, header, copy_chars);
        firstline[copy_chars] = '\0';
        return 0;
    }
    return -1;
}


void
test_getfirstline() {
    char header[] = "GET /path/to/resource HTTP/1.1\r\nHost: foo.com\r\n";
    char firstline[100];
    eqint(0, getfirstline(header, firstline, sizeof(firstline)));


    /* Case: first line is bigger than buffer. */
    char longheader[] = "GET /path/to/resource HTTP/1.1\nHost: example.com\n";
    char shortfirstline[10] = "";
    eqint(0, getfirstline(longheader, shortfirstline,
                sizeof(shortfirstline)));
}


void
test_getfirstline_error() {
    char noline_header[] = "POST /path/to/resource HTTP/1.1";
    char firstline[100] = "";
    eqint(-1, getfirstline(noline_header, firstline, sizeof(firstline)));

    char emptyheader[] = "";
    eqint(-1, getfirstline(emptyheader, firstline, sizeof(firstline)));
}


int
getheaders(char *header, char *content, size_t maxlen) {
    const char *start = strstr(header, "\r\n") + 2;
    const char *end = strstr(header, "\r\n\r\n");

    if (start != NULL && end != NULL) {
        size_t copy_chars = end - start;

        if (copy_chars > maxlen - 1) {
            copy_chars = maxlen - 1;
        }

        strncpy(content, start, copy_chars);
        content[copy_chars] = '\0';
        return 0;
    }
    return -1;
}


void
test_getheaders() {
    char header[] = "POST /path/to/resource HTTP/1.1\r\n"
            "Host: foo.com\r\n\r\nbarbazquxquux.";
    char content[100] = "";
    eqint(0, getheaders(header, content, sizeof(content)));
    eqstr(content, "Host: foo.com");
    eqint(strlen(content), 13);


    char complex_header[] = "POST /path/to/resource HTTP/1.1\r\n"
            "Host: foo.com\r\nAccept: bar\r\n\r\nThis is the content.";
    char complex_content[100] = "";
    eqint(0, getheaders(complex_header, complex_content,
            sizeof(complex_content)));
    eqstr(complex_content, "Host: foo.com\r\nAccept: bar");
    eqint(strlen(complex_content), 26);

    /* Case: header is bigger than buffer. */
    char longheader[] = "GET /path/to/resource HTTP/1.1\r\n"
            "Host: foo.com\r\n\r\nThis is the content.";
    char shortcontent[10] = "";
    eqint(0, getheaders(longheader, shortcontent, sizeof(shortcontent)));
    eqstr(shortcontent, "Host: foo");
}


void
test_getheaders_error() {
    char invalid_header[] = "GET /path/to/resource HTTP/1.1\r\n"
            "Host: example.com\r\n";
    char content[100] = "";
    eqint(-1, getheaders(invalid_header, content, sizeof(content)));
}


int
reqparser(char *request, struct chttpd_request *conn) {
    char firstline[HEADERLINE_MAXSIZE];
    if (getfirstline(request, firstline, HEADERLINE_MAXSIZE) != 0) {
        return -1;
    }

    char *saveptr = NULL;
    if (reqtok(firstline, &saveptr, &conn->verb, &conn->path, &conn->version)
            == -1) {
        return -1;
    }

    char headers[HEADERLINE_MAXSIZE * HEADERS_MAXCOUNT];
    if (getheaders(request, headers, HEADERLINE_MAXSIZE * HEADERS_MAXCOUNT)
            != 0) {
        return -1;
    }
    strcpy(conn->headers, headers);

    headparser(request, conn->accept, conn->connection, conn->content_length,
            conn->content_type);

    return 0;
}


void
test_reqparser() {
    char request[] = "GET /foo/bar HTTP/1.1\r\n"
            "Accept: baz\r\nConnection: corge\r\nContent-Type: qux/quux\r\n"
            "Host: foohost\r\n\r\n";

    struct chttpd_request *conn;
    conn = (struct chttpd_request*)malloc(sizeof(struct chttpd_request));

    eqint(0, reqparser(request, conn));
    // eqstr("GET", conn->verb);
    // eqstr("/foo/bar", conn->path);
    // eqstr("1.1", conn->version);
    eqstr(conn->accept, "baz");
    eqstr(conn->connection, "corge");
    eqstr(conn->content_type, "qux/quux");
    eqstr(conn->content_length, "");
    eqstr(conn->headers, "Accept: baz\r\n"
            "Connection: corge\r\nContent-Type: qux/quux\r\nHost: foohost");
}


void
main() {
    test_headparser();
    test_headparser_error();

    test_reqtok();
    test_reqtok_error();

    test_getfirstline();
    test_getfirstline_error();

    test_getheaders();
    test_getheaders_error();

    test_reqparser();
}
