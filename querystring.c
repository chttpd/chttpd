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
#include <string.h>


/**
 * @brief This function decodes a URL-encoded string.
 *        It takes a pointer to the URL-encoded string, and decodes it in an
 *        in-place manner.
 *
 * @param encoded The URL-encoded string
 */
int
qsdecode(char *encoded) {
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


/**
 * @brief This function is responsible for parsing querystring.
 *        Returns -1 on error, returns 0 on success which means you must stil
 *        call to get the subsequent key/value back, and returns 1 indicating
 *        end of querystring.
 *
 * @param query Pointer to the query in memory.
 * @param saveptr See strtok_r().
 * @param key Representing the key of querystring.
 * @param value Representing the value of querystring.
 */
int
qstok(char *query, char **saveptr, char **key, char **value) {
    if ((query != NULL) && (*saveptr != NULL)) {
        /* **saveptr must be null in the first try */
        return -1;
    }

    if ((*saveptr == NULL) && (query == NULL)) {
        /* query is not provided to qstok. */
        return -1;
    }

    if ((key == NULL) || (value == NULL)) {
        /* key/value provided to qstok cannot be NULL. */
        return -1;
    }

    *key = strtok_r(query, "=", saveptr);
    *value = strtok_r(NULL, "&", saveptr);

    if ((*key == NULL) && (*value == NULL)) {
        return 1;
    }

    if ((qsdecode(*key) != 0) || (qsdecode(*value) != 0)) {
        return -1;
    }

    return 0;
}
