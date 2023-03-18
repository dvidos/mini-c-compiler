#include <stdio.h>

struct a {
    int a: 5;
    int b: 3;
};

struct a func1() {
    struct a b;
    b.a = 1;
    b.b = 2;
    return b;
}

void main() {
   struct a x = func1();
   printf("a.a = %d, a.b = %d\n", x.a, x.b);

}

