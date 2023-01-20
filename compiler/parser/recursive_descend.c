#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "../defs.h"
#include "../token.h"
#include "../ast_node.h"
#include "iterator.h"
#include "shunting_yard.h"

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

// three main AST types:
// - declaration (variable, function)
// - statement (take action: loop, jump, return)
// - expression (something to be evaluate and produce a value, includes function calls)

int parse_statement();
int parse_block(); // parses statements in a loop


static bool is_type_declaration() {
    // storage_class_specifiers: typedef, extern, static, auto, register.
    // type_qualifiers: const, volatile.
    // type specifiers: void, char, short, int, long, float, double, 
    //                  signed, unsigned, <struct/union/enum>, <type-name>
    // keeping it simple for now
    return (next_is(TOK_INT)
         || next_is(TOK_CHAR));
}

static bool accept_type_declaration() {
    if (!is_type_declaration())
        return false;

    consume();
    return true;
}

int parse_statement() {
    if (is_type_declaration()) {
        // it's a declaration of a variable or function declaration or definition
        accept_type_declaration();
        expect(TOK_IDENTIFIER);
        if (accept(TOK_LPAREN)) {
            // function call or declaration
            // parse function args (expressions with commas)

            expect(TOK_RPAREN);
        }
        if (accept(TOK_ASSIGNMENT)) {
            // a variable and a value
            parse_expression_using_shunting_yard();
            expect(TOK_END_OF_STATEMENT);
        }
    }
    if (accept(TOK_IF)) {
        expect(TOK_LPAREN);
        parse_expression_using_shunting_yard();
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
        parse_expression_using_shunting_yard();
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
            parse_expression_using_shunting_yard();
            expect(TOK_END_OF_STATEMENT);
        }
    }
    if (accept(TOK_IDENTIFIER)) {
        // can be declaration, function call or assignment
        // etc.
    }
    if (accept(TOK_CONTINUE)) {
        // a continue keyword
    }
    if (accept(TOK_BREAK)) {
        // a break keyword
    }
}

int parse_block() {
    while (!parsing_failed() && !next_is(TOK_BLOCK_END) && !next_is(TOK_EOF)) {
        parse_statement();
    }
}

int parse_function_arguments_list() {
    while (!next_is(TOK_RPAREN)) {
        expect(TOK_INT); // type
        expect(TOK_IDENTIFIER); // name
        if (accept(TOK_COMMA))
            continue;
        else
            break;
    }
}

int parse_file_using_recursive_descend() {
    // table level indentifiers: static, typedef, <type>, extern, etc.
    
    while (!parsing_failed() && !next_is(TOK_EOF)) {
        expect(TOK_INT);
        expect(TOK_IDENTIFIER);
        if (accept(TOK_ASSIGNMENT)) {
            // variable with initial value
            parse_expression_using_shunting_yard();
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

    return parsing_failed() ? ERROR : SUCCESS;
}

