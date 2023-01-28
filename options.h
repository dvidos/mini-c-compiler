#pragma once
#include <stdbool.h>

struct options {
    bool verbose;
    char *filename;
};

extern struct options options;

void parse_options(int argc, char *argv[]);

