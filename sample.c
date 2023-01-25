// /*
//     This is a sample comment.
//     It exceeds in many lines.
//     Thank you.
//  */

int simple_var;
int var_with_value = 123;

// this is a comment
int add_numbers(int a, int b) {
    int a = 1, 2, 3;
    int c = 2 * 3 + 4; // 10
    int d = 2 + 3 * 4; // 14
    int e = (2 + 3) * 4;    // 5 * 4
    int f1 = 1 + 2 + 3 + 4;  // 10
    int f2 = 1 * 2 + 3 * 4;  // 2 + 12
    int f3 = 1 + 2 * 3 + 4;  // 1 + 6 + 4
    int g = a;
    int g = a + 1;
    int g = a + c * d;
    int k = 123456;
    int k = "asdf";
    int k = '4';
}

int flow_demonstration(int a) {

    while (a < 100) {
        if (a % 2 == 0) {
            a = a * 2 + 1;
        } else {
            a = a + 3;
        }

        if (a < 50) {
            a++;
            continue;
        }

        if (a > 101)
            break;
    }

    return a + 5;
}

int precedence_demonstration() {
    int a = 1 + 2 * 3;
    int b = 1 - 2 / 3;
    int c = 1 + -2 % 3;
    int d = 3 >> 4 + 1;
    int e = d < 1 << 5;
    int f = e == d < 1;
    int g = 0x1F & a == b; // == is higher
    int h = 1 ^ 2 & 3;
    int i = 1 | 2 ^ 3;
    int j = a && b | 0xFF;
    int k = a || b && c;
}

int fibonacci(int num) {
    if (num <= 2)
        return num;
    return fibonacci(num - 1) + fibonacci(num - 2);
}

int factorial(int num) { 
    if (num <= 1)
        return 1;
    
    return num * factorial(num - 1);
}


// this is another comment
void main() {
    int a = 1 ^ 2 * 3 + 4 > 5 & 7 / 8 || odd && even;
    a = add_numbers(5, 2, a, b);
    int response = flow_demonstration(5);

    int num = 0;
    while (num++ <= 10) {
        fibonacci(num);
    }
}

