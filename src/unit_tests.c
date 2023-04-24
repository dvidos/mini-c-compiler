#include <stdio.h>
#include "unit_tests.h"

#ifdef INCLUDE_UNIT_TESTS

static int assertions_made = 0;
static int assertions_failed = 0;

bool unit_tests_outcome() {
    if (assertions_failed == 0) {
        fprintf(stderr, "\n** Unit Tests: PASSED ** (%d assertions)\n", assertions_made);
        return true;
    } else {
        fprintf(stderr, "\n** Unit Tests: FAILED ** (%d assertions failed, out of %d total)\n",
            assertions_failed, assertions_made);
        return false;
    }
}

void unit_tests_assert(bool cond, char *expr, char *filename, int line) {
    assertions_made++;
    fprintf(stderr, ".");

    if (!cond) {
        assertions_failed++;
        fprintf(stderr, "\nAssertion failed: \"%s\", at %s:%d", expr, filename, line);
    }
}

#endif // ifdef INCLUDE_UNIT_TESTS
