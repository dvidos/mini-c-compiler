#include <stdio.h>

char *msg = "Hello world";
extern int a;


void greeting() {
    printf("%s, the sum is %d\n", msg, a + 1);

}


