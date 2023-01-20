/*
    The major pieces are the following:
    - declaration (of variables, functions etc. name, type and value of a symbol)
    - statement (a piece of work to be executed, e.g. loops, branches, returns)
    - expression (a piece of calculation to be evaluated)

    Each element has a "next" pointer to allow to build lists
*/

struct type;
struct declaration;
struct expression;
struct statement;

typedef enum type_family {
    DT_INT,
    DT_CHAR,
    DT_BOOL,
    DT_VOID,
} type_family;

typedef enum expression_type {
    EX_LITERAL_VALUE,
    EX_ADDITION,
    EX_SUBTRACTION,
    EX_MULTIPLICATION,
    EX_DIVISION,
} expression_type;

typedef enum statement_type {
    ST_DECL,
    ST_EXPRESSION, // e.g. function call
    ST_ASSIGNMENT,
    ST_IF,
    ST_WHILE,
    ST_CONTINUE,
    ST_BREAK,
    ST_RETURN,
} statement_type;


// type of a variable or symbol
struct type {
    type_family family; // int, char, etc
    int bits; // array-of or pointer-to? signed, unsigned, const etc

    struct type *next;
};

// declaration of a variable or function
struct declaration {
    char *name;
    struct type *type;
    struct expression *value; // optional for initialization
    struct statement *statements; // e.g. for functions

    struct declaration *next;
};

// as simple as a literal num, or as complex as a tree
struct expression {
    // can be even a literal, a function call, operations etc.
    // can be a tree of expressions
    expression_type type;
    struct expression *left; // or single, e.g. for the NOT expression
    struct expression *right;
};

struct statement {
    // statement type?
    // assignment, for, while, if, return, etc
    // ifs have one condition and two bodies, if and else
    // whiles have one condition & one body
    statement_type type;
    

    struct statement *next;
};

