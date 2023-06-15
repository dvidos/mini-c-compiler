#include <stdio.h>

char *msg = "Hello world";
extern int a;


void greeting1() {
    printf("%s, the sum is %d\n", msg, a + 1);

}

void greeting2() {
    printf("%s, from greeting2, the sum is %d\n", msg, a + 2);

}


