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
#include <clog.h>

#include "chttpd.h"
#include "helpers.c"
#include "connection.c"
#include "body.c"
#include "form.c"
#include "request.c"
#include "response.c"

#include "testhelpers.h"


static ASYNC
assert_formfields(struct caio_task *self, struct chttpd_connection *req) {
    struct chttpd_formfield *field = NULL;
    int flags = 0;
    CAIO_BEGIN(self);

    DEBUG("%p", req->form->nextfield);
    CHTTPD_FORMFIELD_NEXT(self, req, &field, flags);
    eqint(0, self->eno);
    isfalse(req->closing);
    isnotnull(field);
    isnotnull(field->name);
    isnotnull(field->u.value);
    eqstr("foo", field->name);
    eqstr("bar", field->u.value);

    CAIO_FINALLY(self);
}


void
test_request_form_urlencoded() {
    const char *in;
    struct chttpd_connection req;
    memset(&req, 0, sizeof(req));
    struct tfile infile = tmpfile_open();
    req.fd = infile.fd;
    istrue(req.fd > 2);
    req.inbuff = mrb_create(CHTTPD_REQUEST_HEADER_BUFFSIZE);
    req.outbuff = mrb_create(CHTTPD_REQUEST_HEADER_BUFFSIZE);
    chttpd_request_reset(&req);

    /* Complex request */
    REQ("GET /foo/bar?foo=bar&baz=qux HTTP/1.1\r\n"
        "Connection: close\r\n"
        "Content-Type: application/x-www-form-urlencoded\r\n"
        "Content-Length: 15\r\n\r\n"
        "foo=bar&baz=qux\r\n");
    eqint(0, chttpd_request_parse(&req));

    eqint(0, chttpd_form_new(&req));
    CAIO_FOREVER(assert_formfields, &req, 1);

    chttpd_request_reset(&req);
    mrb_destroy(req.inbuff);
    mrb_destroy(req.outbuff);
    fclose(infile.file);
}


int
main() {
    test_request_form_urlencoded();
    return EXIT_SUCCESS;
}
