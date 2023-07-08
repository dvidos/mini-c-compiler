#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

int warnings_count = 0;
int errors_count = 0;

static void print_stderr(const char *filename, int line_no, char *severity, char *msg, va_list args) {
    errors_count++;
    
    if (filename != NULL) {
        fprintf(stderr, "%s:", filename);
        if (line_no > 0)
            fprintf(stderr, "%d:", line_no);
        fprintf(stderr, " ");
    }
    
    fprintf(stderr, "%s: ", severity);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
}

void warn_at(const char *filename, int line_no, char *msg, ...) {
    warnings_count++;

    va_list args;
    va_start(args, msg);
    print_stderr(filename, line_no, "warning", msg, args);
    va_end(args);
}

void error_at(const char *filename, int line_no, char *msg, ...) {
    errors_count++;

    va_list args;
    va_start(args, msg);
    print_stderr(filename, line_no, "error", msg, args);
    va_end(args);
}

void error(char *msg, ...) {
    errors_count++;

    va_list args;
    va_start(args, msg);
    print_stderr(NULL, 0, "error", msg, args);
    va_end(args);
}

void fatal(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    print_stderr(NULL, 0, "fatal", msg, args);
    va_end(args);

    exit(1);
}

