// Copyright 2025 Vahid Mardani
/*
 * This file is part of carrot.
 *  carrot is free software: you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation, either version 3 of the License, or (at your option)
 *  any later version.
 *
 *  carrot is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with carrot. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Author: Vahid Mardani <vahid.mardani@gmail.com>
 */
/* thirdparty */
#include <pcaio/pcaio.h>
#include <pcaio/modio.h>
#include <pcaio/modepoll.h>

/* local public */
#include "carrot/carrot.h"


int
carrot_main(struct carrot *s) {
    int ret;
    struct pcaio_task *task;
    struct pcaio_iomodule *modepoll;

    /* register modules and tasks */
    if (pcaio_modepoll_use(16, &modepoll)) {
        return -1;
    }

    if (pcaio_modio_use(modepoll)) {
        return -1;
    }

    // TODO: task status
    task = pcaio_task_new(carrotA, NULL, 1, s);
    if (task == NULL) {
        return -1;
    }

    /* run event loop */
    // TODO: only reflect the pcaio function result, and return task status
    //       instead
    ret = pcaio(1, &task, 1);
    carrot_free(s);

    return ret;
}
