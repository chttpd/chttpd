#include <stdlib.h>

#include <clog.h>
#include <carrow.h>

#include <chttpd.h>


#define PAGESIZE 4096
#define BUFFSIZE (PAGESIZE * 32768)


int
main() {
    clog_verbosity = CLOG_DEBUG;

    if (carrow_handleinterrupts()) {
        return EXIT_FAILURE;
    }

    struct chttpd state = {
        .bindaddr = "0.0.0.0",
        .bindport = 3030,
        .backlog = 2,
        .buffsize = BUFFSIZE,
    };

    return chttpd_forever(chttpdA, &state, NULL);
}
