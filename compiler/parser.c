#include <stddef.h>
#include <stdio.h>
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
 */

// let's try a Recursive Descend Parser
// if operator precedence is too high, we can switch to other parsers.
// we need a ton of tests, to verify things!!!


static token *iterator_ptr;

// returns current token's type
static token_type current() {
    return iterator_ptr->type;
}

static void next_token() {
    iterator_ptr = iterator_ptr->next;
}

// consumes token, only if matching. 
static bool accept(token_type type) {
    if (iterator_ptr->type != type)
        return false;
    
    next_token();
    return true;
}

// errors if token does not match
static bool expect(token_type type) {
    if (!accept(type)) {
        printf("Was expecting %s token, but got %s instead", 
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
    do {
        parse_statement();
    } while (current() != TOK_BLOCK_END && current() != TOK_EOF);
}

int parse_expression() {
    do {
        if (accept(TOK_LPAREN)) {
            // nested expression
            parse_expression();
            expect(TOK_RPAREN);
        } else {
            // maybe prefix operators, such as &, *, !, ~
        }
    } while (current() != TOK_END_OF_STATEMENT && current() != TOK_RPAREN);
}

int parse_function_arguments_list() {
    while (current() != TOK_RPAREN) {
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
    
    while (current() != TOK_EOF) {
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

