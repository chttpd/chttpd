#include <stdlib.h>

#include <cutest.h>


void
test_querystring() {
    eqint(2, 2);
    eqint(2, 2);
    eqint(2, 2);
    eqint(2, 2);
    eqint(2, 1);
}


int
main() {
    test_querystring();
    return EXIT_SUCCESS;
}
