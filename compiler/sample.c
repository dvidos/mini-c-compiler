/*
    This is a sample comment.
    It exceeds in many lines.
    Thank you.
*/

int simple_var;
int var_with_value;

// this is a comment
int add_numbers(int a, int b) {
    int c = 2 * 3 + 4; // 10
    int d = 2 + 3 * 4; // 14
    int e = (2 + 3) * 4;    // 5 * 4
    int f = 1 + 2 + 3 + 4;  // 10
    int f = 1 * 2 + 3 * 4;  // 2 + 12
    int f = 1 + 2 * 3 + 4;  // 1 + 6 + 4

    if (c > 3) {
        return 1;
    } else {
        return c;
    }

    int x = 3;
    while (x > 0) {
        // something
        if (x > 10)
            continue;
        if (x < 3)
            break;
    }
}

// this is another comment
void main() {
    // add_numbers(5, 2);
}

