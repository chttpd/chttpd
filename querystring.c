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

#include "helpers.h"


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
chttpd_querystring_tokenize(char *query, char **saveptr, char **key,
        char **value) {
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

    if ((urldecode(*key) != 0) || (urldecode(*value) != 0)) {
        return -1;
    }

    return 0;
}
