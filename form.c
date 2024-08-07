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

#include "chttpd.h"
#include "form.h"
#include "helpers.h"


#undef CAIO_ARG1
#undef CAIO_ARG2
#undef CAIO_ENTITY
#define CAIO_ENTITY chttpd_form
#define CAIO_ARG1 struct chttpd_formfield **
#define CAIO_ARG2 int
#include "caio/generic.c"


static ASYNC
form_urlencodedA(struct caio_task *self, struct chttpd_form *form,
        struct chttpd_formfield **, int flags) {
    CAIO_BEGIN(self);

    /* Ensure the entire body is already received. */
    if (form->req->remainingbytes != 0) {
        AWAIT(self, chttpd_connection, body_readA, form->req);
        // if (CAIO_HASERROR(self)
    }

    // /* Tokenize the body */
    // chttpd_querystring_tokenize(char *query, char **saveptr, char **key,

    CAIO_FINALLY(self);
}


int
chttpd_form_new(struct chttpd_connection *req) {
    struct chttpd_form *form;

    if (req->form) {
        ERRNO_SET(CHTTPD_ENO_FORMALREADYINITIALIZED);
        return -1;
    }

    form = malloc(sizeof(struct chttpd_form));
    if (form == NULL) {
        ERRNO_SET(CHTTPD_ENO_OUTOFMEMORY);
        return -1;
    }

    req->form = form;
    form->req = req;

    if (STARTSWITH(req->contenttype, HTTP_CONTENTTYPE_FORM_URLENCODED)) {
        form->type = CHTTPD_FORMTYPE_URLENCODED;
        form->nextfield = form_urlencodedA;
    }
    // else if (STARTSWITH(req->contenttype, HTTP_CONTENTTYPE_FORM_MULTIPART)) {
    //     form->type = CHTTPD_FORMTYPE_MULTIPART;
    //     form->nextfield = chttpd_form_multipart_next;
    // }
    // else if (STARTSWITH(req->contenttype, HTTP_CONTENTTYPE_FORM_MULTIPART)) {
    //     form->type = CHTTPD_FORMTYPE_JSON;
    //     form->nextfield = chttpd_form_json_next;
    // }
    else {
        form->type = CHTTPD_FORMTYPE_UNKNOWN;
        form->nextfield = NULL;
        return -1;
    }
    return 0;
}


void
chttpd_form_dispose(struct chttpd_connection *req) {
    if (req->form == NULL) {
        return;
    }

    free(req->form);
}
