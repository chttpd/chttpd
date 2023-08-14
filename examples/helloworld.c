#include <stdlib.h>

#include <clog.h>
#include <carrow.h>

#include <chttpd.h>


int
main() {
    clog_verbosity = CLOG_DEBUG;

    if (carrow_handleinterrupts()) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
