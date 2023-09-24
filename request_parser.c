#include <ctype.h>

#include "chttpd.h"
#include "request_parser.h"


static char *
trim(char *s) {
    if (s == NULL) {
        return NULL;
    }
    int l = strlen(s);

    while (s[0] && isspace(s[0])) {
        s++;
        l--;
    }

    while (isspace(s[l -1])) {
        s[--l] = 0;
    }

    return s;
}


const char *
chttpd_request_header_get(struct chttpd_request *req, const char *name) {
    int i;
    const char *header;

    for (i = 0; i < req->headerscount; i++) {
        header = req->headers[i];
        if (strcasestr(header, name) == header) {
            return trim((char *)(header + strlen(name) + 1));
        }
    }

    return NULL;
}


int
chttpd_request_parse(struct chttpd_request *req, char *header,
        int headerlen) {
    char *saveptr;
    char *linesaveptr;
    char *line;
    char *token;

    /* Preserve header and it's len */
    req->header = header;
    req->headerlen = headerlen;
    req->headerscount = 0;

    /* Protocol's first line */
    line = strtok_r(header, "\r\n", &saveptr);
    if (line == NULL) {
        goto failed;
    }

    /* Verb */
    token = strtok_r(line, " ", &linesaveptr);
    if (token == NULL) {
        goto failed;
    }

    /* Initialize the request fields */
    req->verb = token;
    req->contentlength = -1;

    /* Path */
    token = strtok_r(NULL, " ", &linesaveptr);
    if (token == NULL) {
        goto failed;
    }
    req->path = token;

    /* HTTP version */
    token = strtok_r(NULL, "/", &linesaveptr);
    if (token) {
        req->version = token;
        token = strtok_r(NULL, "\r\n", &linesaveptr);
        if (token) {
            req->version = token;
        }
    }
    else {
        req->version = NULL;
    }

    /* Read headers */
    while ((line = strtok_r(NULL, "\r\n", &saveptr))) {
        if (strcasestr(line, "connection:") == line) {
            req->connection = trim(line + 11);
        }
        else if (strcasestr(line, "content-type:") == line) {
            req->contenttype = trim(line + 13);
        }
        else if (strcasestr(line, "content-length:") == line) {
            req->contentlength = atoi(trim(line + 15));
        }
        else if (req->headerscount < (CHTTPD_REQUESTHEADERS_MAX - 1)) {
            req->headers[req->headerscount++] = line;
        }
        else {
            goto failed;
        }
    }

    return 0;

failed:
    return -1;
}
