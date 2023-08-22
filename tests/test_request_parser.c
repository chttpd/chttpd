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


struct chttpd_request {
    char *verb;
    char *path;
    char *version;
    char *content_length;
    char *content_type;
    char *connection;
    char *accept;
    char *headers;
}


int
headtok(char *headers, char **saveptr, char **key, char **value) {
    if ((headers != NULL) && (*saveptr != NULL)) {
        /* **saveptr must be null in the first try */
        return -1;
    }

    if ((*saveptr == NULL) && (headers == NULL)) {
        /* headers is not provided to headtok. */
        return -1;
    }

    if ((key == NULL) || (value == NULL)) {
        /* key/value provided to headtok cannot be NULL. */
        return -1;
    }

    *key = strtok_r(headers, ":", saveptr);
    *value = strtok_r(NULL, "\r\n", saveptr);

    if ((*key == NULL) || (*value == NULL)) {
        return 1;
    }

    while (((**key) == ' ') || ((**key) == '\r') || ((**key) == '\n')) {
        (*key)++;
    }

    while ((**value) == ' ' || ((**value) == '\r') || ((**value) == '\n')) {
        (*value)++;
    }

    return 0;
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

    char *saveptr;
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


void
test_headtok() {
    const char *request = "Host: foo.bar\r\nContent-Type: baz/qux\r\n"
        "Connection: corge\r\nContent-Length: 99\r\n";

    char *copy = malloc(strlen(request) + 1);
    strcpy(copy, request);

    char *saveptr = NULL;
    char *key;
    char *value;

    eqint(0, headtok(copy, &saveptr, &key, &value));
    eqstr(key, "Host");
    eqint(strlen(key), 4);
    eqstr(value, "foo.bar");
    eqint(strlen(value), 7);

    eqint(0, headtok(NULL, &saveptr, &key, &value));
    eqstr(key, "Content-Type");
    eqint(strlen(key), 12);
    eqstr(value, "baz/qux");
    eqint(strlen(value), 7);

    eqint(0, headtok(NULL, &saveptr, &key, &value));
    eqstr(key, "Connection");
    eqint(strlen(key), 10);
    eqstr(value, "corge");
    eqint(strlen(value), 5);

    eqint(0, headtok(NULL, &saveptr, &key, &value));
    eqstr(key, "Content-Length");
    eqint(strlen(key), 14);
    eqstr(value, "99");
    eqint(strlen(value), 2);

    eqint(1, headtok(NULL, &saveptr, &key, &value));
}


void
test_headtok_error() {
    const char *request = "Host: foo.bar\r\nContent-Type:: baz/qux\r\n"
        "Connection: corge\r\nContent-Length: 99\r\n";

    char *copy = malloc(strlen(request) + 1);
    strcpy(copy, request);

    char *saveptr = NULL;
    char *key;
    char *value;

    eqint(0, headtok(copy, &saveptr, &key, &value));
    eqint(-1, headtok(copy, &saveptr, &key, &value));
    saveptr = copy;
    eqint(-1, headtok(copy, &saveptr, &key, &value));
    saveptr = NULL;
    eqint(-1, headtok(NULL, &saveptr, NULL, &value));
    eqint(-1, headtok(NULL, &saveptr, &key, NULL));
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


int
reqparser(char *request, chttpd_request *conn) {
    char firstline[HEADERLINE_MAXSIZE];
    if (get_first_line(request, HEADERLINE_MAXSIZE) != 0) {
        return -1;
    }

    char *saveptr = NULL;
    if (reqtok(firstline, &saveptr, &verb, &path, &version) == -1) {
        CORO_REJECT("Request tokenization failed.");
    }
            
    char headers[HEADERLINE_MAXSIZE];
    if (get_headers(request, headers, HEADERLINE_MAXSIZE) != 0) {
        return -1;
    }
    // strcpy(conn->headers, headers);

    saveptr = NULL;
    int headtok_resp;
    char *key;
    char *value;
    
    int i = 0;
    do {
        if (i == 0) {
            headtok_resp = headtok(headers, &saveptr, &key, &value);
        }
        else {
            /* Ensure headtok recieves NULL in the proceeding calls */
            headtok_resp = headtok(NULL, &saveptr, &key, &value);
        }
        
        if (headtok_resp == -1) {
            return -1;
        }

        if (headtok_resp == 1) {
            break;
        }
        
        i++;
    } while (headtok_resp == 0);

}


void
test_reqparser() {
    char request[1024] = "GET /foo/bar HTTP/1.1\r\n"
        "Accept: baz\r\n"
        "Connection: corge\r\n"
        "Content-Type: qux/quux\r\n"
        "Host: foohost\r\n"
        "\r\n"
    
    struct *chttpd_request conn;
    reqparser(request, conn);

    eqstr("GET", conn->verb);
    eqstr("/foo/bar", conn->path);
    eqstr("1.1", conn->versoin);
    eqstr(conn->accept, "baz");
    eqstr(conn->connection, "corge");
    eqstr(conn->content_type, "qux/quux");
    eqstr(conn->content_length, NULL);
    eqstr(conn->headers, "Accept: baz\r\n"
        "Connection: corge\r\n"
        "Content-Type: qux/quux\r\n"
        "Host: foohost\r\n"
        "\r\n"
    );
}

void
main() {
    test_reqtok();
    test_reqtok_error();
    test_headtok();
    test_headtok_error();

    // test_request_parser();
}
