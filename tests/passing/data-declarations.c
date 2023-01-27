
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

