/*

// if we do not have this declaration, we get a "symbol not defined"
void printf(char *fmt, int a, int b);

int counter = 65534;
char *message = "Hello world!";
int numbers[10];


int get_next_counter_value() {
    counter = counter + 1;
    return counter;
}

int rect_area(int width, int height) {
    return width * height;
}

int triangle_area(int width, int height) {
    return width * height / 2;
}

int circle_area(int radius) {
    return 3 * radius * radius; // we don't do floating for now
}

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
*/

void main(int argc, char **argv) {
    int a = 1;
    int b;
    char c;
    int *d;
    char buffer[16];

    b = a + 2;
}
