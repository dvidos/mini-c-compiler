#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

int errors_count = 0;

void error(char *filename, int line_no, char *msg, ...) {
    errors_count++;
    
    if (filename != NULL)
        printf("%s:", filename);
    if (line_no > 0)
        printf("%d:", line_no);
    
    printf(" error: ");

    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);

    printf("\n");
}

