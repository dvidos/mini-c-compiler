
// if we do not have this declaration, we get a "symbol not defined"
void printf(char *fmt, int a, int b);


int fibonacci(int n) {
    if (n <= 2)
        return n;
    
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int factorial(int n) {
    if (n <= 1)
        return n;

    return n * factorial(n - 1);
}

void math_demo() {
    int i;

    i = 0; 
    while (i++ < 10) {
        printf("fibonacci of %d is %d\n", i, fibonacci(i));
    }

    i = 0; 
    while (i++ < 10) {
        printf("factorial of %d is %d\n", i, factorial(i));
    }
}

int main(int argc, char **argv) {
    math_demo();
    return 0;
}
