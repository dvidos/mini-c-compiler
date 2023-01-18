#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "defs.h"
#include "token.h"

/**
 * How to parse an expression into a tree????
 * Sounds like the shunting-yard algorithm is in order
 * It uses a stack to push things in natural (source code) order, 
 * but pop in RPN (reverse polish notation)
 * Invented by Dijkstra of course!
 * Oh... there's a lot to learn here, actally...
 * Parsers, LL, LR, recursive, etc.
 * I think I'll need a Recursive descent parser in general,
 * but still want to get to the bottom of parsing an expression with precedence.
 * It seems Recursive parser is ok for some levels of precedence, but,
 * for many levels, coding becomes tedius, as the sequence of calling the functions
 * is what determines precedence.
 * For a true multi-precedence parser, I should look into 
 * 
 * See 
 * - https://rosettacode.org/wiki/Parsing/Shunting-yard_algorithm
 * - https://en.wikipedia.org/wiki/Recursive_descent_parser
 * - https://www.lysator.liu.se/c/ANSI-C-grammar-y.html
 * - https://www.tutorialspoint.com/cprogramming/c_operators.htm
 */

// let's try a Recursive Descend Parser
// if operator precedence is too high, we can switch to other parsers.
// we need a ton of tests, to verify things!!!


static token *iterator_ptr;
static bool failed = false;


static void fail(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    vprintf(args, msg);
    va_end(args);
    failed = 1;
}

static bool reached(token_type type) {
    return iterator_ptr->type == type;
}

static void go_next() {
    iterator_ptr = iterator_ptr->next;
}

// consumes token, only if matching. 
static bool accept(token_type type) {
    if (iterator_ptr->type != type)
        return false;
    
    go_next();
    return true;
}

// errors if token does not match
static bool expect(token_type type) {
    if (!accept(type)) {
        fail("Was expecting %s token, but got %s instead", 
            token_type_name(type),
            token_type_name(iterator_ptr->type)
        );
        return false;
    }

    return true;
}

// ------------------------------------

// in general, parsers do not consume the closing token
// but test against it to see if they finished.
// so, expressions stop when they see a ")" and so do blocks for the "}"
int parse_statement();
int parse_block();
int parse_expression();


int parse_statement() {
    if (accept(TOK_IF)) {
        expect(TOK_LPAREN);
        parse_expression();
        expect(TOK_RPAREN);
        if (accept(TOK_BLOCK_START)) {
            parse_block();
            expect(TOK_BLOCK_END);
        } else {
            parse_statement();
            expect(TOK_END_OF_STATEMENT);
        }
        if (accept(TOK_ELSE)) {
            if (accept(TOK_BLOCK_START)) {
                parse_block();
                expect(TOK_BLOCK_END);
            } else {
                parse_statement();
                expect(TOK_END_OF_STATEMENT);
            }
        }
    }
    if (accept(TOK_WHILE)) {
        expect(TOK_LPAREN);
        parse_expression();
        expect(TOK_RPAREN);
        if (accept(TOK_BLOCK_START)) {
            parse_block();
            expect(TOK_BLOCK_END);
        } else {
            parse_statement();
            expect(TOK_END_OF_STATEMENT);
        }
    }
    if (accept(TOK_RETURN)) {
        if (accept(TOK_END_OF_STATEMENT)) {
            // return with no value
        } else {
            parse_expression();
            expect(TOK_END_OF_STATEMENT);
        }
    }
    if (accept(TOK_IDENTIFIER)) {
        // can be declaration, function call or assignment
        // etc.
    }
}

int parse_block() {
    while (!failed && !reached(TOK_BLOCK_END) && !reached(TOK_EOF)) {
        parse_statement();
    }
}

int is_value() {
    return reached(TOK_LPAREN) 
        || reached(TOK_NUMBER)
        || reached(TOK_STRING_LITERAL)
        || reached(TOK_IDENTIFIER);
}

int parse_value() {
    // here there is a chance for prefix operators
    // e.g. char *p = &variable;
    // or   int i = ++j;

    if (accept(TOK_NUMBER)) {
        // e.g int a = 5;

    } else if (accept(TOK_IDENTIFIER)) {
        // e.g int a = b;

    } else if (accept(TOK_STRING_LITERAL)) {
        // e.g char *a = "hello";

    } else if (accept(TOK_LPAREN)) {
        // e.g. int a = (i + 1);
        parse_expression();
        expect(TOK_RPAREN);
    }
}

int is_operator() {
    return reached(TOK_PLUS_SIGN)
        || reached(TOK_MINUS_SIGN)
        || reached(TOK_STAR)
        || reached(TOK_SLASH);
}

int parse_operator() {
    if (reached(TOK_LPAREN) 
        || reached(TOK_NUMBER)
        || reached(TOK_STRING_LITERAL)
        || reached(TOK_IDENTIFIER)) {
        accept(iterator_ptr->type);
    }
}

int parse_expression() {

    if (is_value())
        parse_value();
    
    while (!failed && !reached(TOK_END_OF_STATEMENT) && !reached(TOK_RPAREN)) {
        if (is_operator())
            parse_operator();
        
        if (is_value())
            parse_value();
    }
}

int parse_function_arguments_list() {
    while (!reached(TOK_RPAREN)) {
        expect(TOK_IDENTIFIER); // type
        expect(TOK_IDENTIFIER); // name
        if (accept(TOK_COMMA))
            continue;
        else
            break;
    }
}

int parse_file(token *first_token) {
    // table level indentifiers: static, typedef, <type>, extern, etc.
    iterator_ptr = first_token;
    failed = false;
    
    while (!failed && !reached(TOK_EOF)) {
        expect(TOK_INT);
        expect(TOK_IDENTIFIER);
        if (accept(TOK_ASSIGNMENT)) {
            // variable with initial value
            parse_expression();
            expect(TOK_END_OF_STATEMENT);
        } else if (accept(TOK_LPAREN)) {
            // function declaration or definition
            parse_function_arguments_list();
            expect(TOK_RPAREN);
            if (accept(TOK_END_OF_STATEMENT)) {
                // just declaration
            } else if (accept(TOK_BLOCK_START)) {
                parse_block();
                expect(TOK_BLOCK_END);
            }
        }
    }
}

