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
#include <cutest.h>


int
decode(char *encoded) {
    size_t length = strlen(encoded);
    size_t i;
    size_t j;

    for (i = 0, j = 0; i < length; i++, j++) {
        if (encoded[i] == '%') {
            if (i + 2 < length) {
                char hex_digits[3] = {encoded[i + 1], encoded[i + 2], '\0'};
                int ascii_value = strtol(hex_digits, NULL, 16);
                encoded[j] = (char)ascii_value;
                i += 2;
            }
            else {
                encoded[j] = '\0';
                return -1;
            }
        }
        else if (encoded[i] == '+') {
            encoded[j] = ' ';
        }
        else {
            encoded[j] = encoded[i];
        }
    }

    encoded[j] = '\0';
    return 0;
}


int
qstok(char *query, char **saveptr, char **key, char **value) {
    if ((query != NULL) && (*saveptr != NULL)) {
        // **saveptr must be null in the first try.
        return -1;
    }

    if ((*saveptr == NULL) && (query == NULL)) {
        // query is not provided to qstok.
        return -1;
    }

    if ((key == NULL) || (value == NULL)) {
        // key/value provided to qstok cannot be NULL.
        return -1;
    }

    *key = strtok_r(query, "=", saveptr);
    *value = strtok_r(NULL, "&", saveptr);

    if ((*key == NULL) && (*value == NULL)) {
        return 1;
    }

    if ((decode(*key) != 0) || (decode(*value) != 0)) {
        return -1;
    }

    return 0;
}


void
test_querystring() {
    const char *query = "foo=bar&baz=quux";

    char *copy = malloc(strlen(query) + 1);
    strcpy(copy, query);

    char *saveptr;
    char *key = NULL;
    char *value = NULL;

    eqint(0, qstok(copy, &saveptr, &key, &value));
    eqint(3, strlen(key));
    eqint(3, strlen(value));
    eqstr("foo", key);
    eqstr("bar", value);

    eqint(0, qstok(NULL, &saveptr, &key, &value));
    eqint(3, strlen(key));
    eqint(4, strlen(value));
    eqstr("baz", key);
    eqstr("quux", value);

    eqint(1, qstok(NULL, &saveptr, &key, &value));
    free(copy);
}


void
test_querystring_error() {
    const char *query = "foo=bar&baz=quux";

    char *copy = malloc(strlen(query) + 1);
    strcpy(copy, query);

    char *saveptr = copy;
    char *key;
    char *value;

    eqint(-1, qstok(copy, &saveptr, &key, &value));
    saveptr = NULL;
    eqint(-1, qstok(NULL, &saveptr, &key, &value));
    eqint(-1, qstok(copy, &saveptr, NULL, &value));
    eqint(-1, qstok(copy, &saveptr, &key, NULL));

    free(copy);
}


void
test_querystring_url_encoded() {
    const char *query = "foo%20bar=baz&qux=quux%21corge";

    char *copy = malloc(strlen(query));
    strcpy(copy, query);

    char *saveptr;
    char *key;
    char *value;

    eqint(0, qstok(copy, &saveptr, &key, &value));
    eqint(7, strlen(key));
    eqint(3, strlen(value));
    eqstr("foo bar", key);
    eqstr("baz", value);

    eqint(0, qstok(NULL, &saveptr, &key, &value));
    eqint(3, strlen(key));
    eqint(10, strlen(value));
    eqstr("qux", key);
    eqstr("quux!corge", value);

    eqint(1, qstok(NULL, &saveptr, &key, &value));
    free(copy);
}


int
main() {
    test_querystring();
    test_querystring_error();
    test_querystring_url_encoded();

    return EXIT_SUCCESS;
}
