
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

// /// @brief 
// /// @return

// int func_no_args() {
//     return 1;
// }

// int func_one_arg(int a) {
//     return 1;
// }

// int func_two_args(int a, char b) {
//     return 1;
// }

// int func_three_args(int a, char b, bool c) {
//     return 1;
// }

// int func_four_args(int a, char b, bool c, int d) {
//     return 1;
// }

// int main() {
//     int x;
//     x = func_no_args();
//     x = func_one_arg(1);
//     x = func_two_args(123, 456);
//     x = func_three_args(123, 456, 789);
//     x = func_four_args(123, 456, 0, 555);
// }
