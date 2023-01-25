/*
    This file part of tests
    Should be parsed succesfully

    Run using `mcc parsing_test.c` and checking the exit code.

    This statement by itself tests the block-style comments
*/

// simple variable types
int a;
float b;
char c;
bool d;
void e; // semantically wrong, but syntactically parseable

char *pa;     // pointer
char **ppa;   // pointer to pointer
// char a[10];   // arrays
// char *a[10];  // arrays of pointers

// simple values with initializer expression
int a = 1;
float b = 1;
char c = 'c';
bool d = true;

// numbers parsing
int a = 123;
int b = -123;   
int c = 0777;   // decimal is 0x1FF or 511
int d = 0x1234; // decimal is 4660
int e = 0X5678; // decimal is 22136

// strings parsing 
char *a = "abcd";
char *b = "";
char *c = "\"she said\", he said";
char *d = "\t <-- tab, and then a new line --> \n";

// // characters
char a = 'a';
char b = '\n';
char c = '\'';
char d = '\0';


void return_demonstration() {
    // simple return
    return;

    // return with value
    return 1;

    // return with expression
    return a > 0;
}

void ifs_demonstration() {

    // sample if without blocks
    if (true)
        return;

    // sample if/else
    if (true)
        return;
    else
        return;

    // if with blocks
    if (true) {
        return;
    }

    // if/else with blocks
    if (true) {
        return;
    } else {
        return;
    }
}

void while_demonstration() {

    // simple while
    while (true)
        return;

    // with blocks
    while (true) {
        return;
    }

    // with break
    while (true) {
        break;
    }

    // with continue
    while (true) {
        continue;
    }

    // all together now
    while (true) {
        if (false) {
            break;
        } else {
            continue;
        }
    }
}

void expressions_demonstration() {
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

void pointers_and_arrays_demonstration() {
    // char a;
    // char *p;

    // a = 'a';
    // p = &a;
    // *p = 'b';

    // char a[10];
    // char b[10];

    // int i = 0;
    // while (i++ < 10)
    //     a[i] = b[i + 1];
}

