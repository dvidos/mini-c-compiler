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

char *pa;             // pointer
char **ppa;           // pointer to pointer
char ar[10];          // array
char ar2[10][20];     // 2-dimensional array
char *arp[10];        // array of pointers
char *ar2p[10][20];   // 2-dimensional array of pointers
char **arpp[10];      // array of pointers to pointers
char **ar2pp[10][20]; // 2-dimensional array of pointers to pointers

// simple values with initializer expression
int   val_a = 1;
float val_b = 1;
char  val_c = 'c';
bool  val_d = true;

// numbers parsing
int num_a = 123;
int num_b = -123;   
int num_c = 0777;   // decimal is 0x1FF or 511
int num_d = 0x1234; // decimal is 4660
int num_e = 0X5678; // decimal is 22136

// strings parsing 
char *str_a = "abcd";
char *str_b = "";
char *str_c = "\"she said\", he said";
char *str_d = "\t <-- tab, and then a new line --> \n";

// // characters
char chr_a = 'a';
char chr_b = '\n';
char chr_c = '\'';
char chr_d = '\0';


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
    int g = 0x1F & a == b;
    int h = 1 ^ 2 & 3;
    int i = 1 | 2 ^ 3;
    int j = a && b | 0xFF;
    int k = a || b && c;
}

void pointers_and_arrays_demonstration() {
    char a;
    char *p;

    a = 'a';
    p = &a;
    *p = 'b';

    char arr_a[10];
    char arr_b[10];

    int i = 0;
    while (i++ < 10)
        arr_a[i] = arr_b[i + 1];
}

