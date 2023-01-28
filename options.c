#include <stdbool.h>
#include <string.h>
#include "options.h"

// global read-only variable
struct options options;


void parse_options(int argc, char *argv[]) {
    memset(&options, 0, sizeof(options));

    char *p;
    for (int i = 1; i < argc; i++) {
        p = argv[i];

        // first letter not a minus
        if (p[0] != '-') {
            options.filename = p;
            continue;
        }

        // first letter a minus
        if (strcmp(p, "-v") == 0) {
            options.verbose = true;
        }
    }
}

