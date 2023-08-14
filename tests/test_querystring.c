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
qstok(char *query, char **saveptr, char **key, char **value);


void
test_querystring() {
    char *saveptr;
    const char *query = "foo=bar&baz=quux";
    char *copy = malloc(strlen(query) + 1);
    strcpy(copy, query);

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
}


void
main() {
    test_querystring();
    test_querystring_error();

    // TODO:
    // test_querystring_encodded();
    return EXIT_SUCCESS;
}
