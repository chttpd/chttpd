// Copyright 2025 Vahid Mardani
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
/* thirdparty */
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>
#include <pcaio/modepoll.h>

/* local public */
#include "chttpd/chttpd.h"


int
chttpd_main(struct chttpd *s) {
    struct pcaio_task *task;
    struct pcaio_iomodule *modepoll;

    /* register modules and tasks */
    if (pcaio_modepoll_use(16, &modepoll)) {
        return -1;
    }

    if (pcaio_modio_use(modepoll)) {
        return -1;
    }

    task = pcaio_task_new((pcaio_taskmain_t)chttpdA, 1, s);
    if (task) {
        return -1;
    }

    /* run event loop */
    return pcaio(1, &task, 1);
}
