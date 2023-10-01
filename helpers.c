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
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


char *
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


/**
 * @brief This function decodes a URL-encoded string.
 *        It takes a pointer to the URL-encoded string, and decodes it in an
 *        in-place manner.
 *
 * @param encoded The URL-encoded string
 */
int
urldecode(char *encoded) {
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
