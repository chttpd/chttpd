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
