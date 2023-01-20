#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "../defs.h"
#include "../token.h"
#include "../ast_node.h"
#include "../ast.h"
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

void parse_statement();
void parse_block(); // parses statements in a loop


static bool is_type_declaration() {
    // storage_class_specifiers: typedef, extern, static, auto, register.
    // type_qualifiers: const, volatile.
    // type specifiers: void, char, short, int, long, float, double, 
    //                  signed, unsigned, <struct/union/enum>, <type-name>
    // keeping it simple for now
    return (next_is(TOK_INT)
         || next_is(TOK_CHAR)
         || next_is(TOK_VOID));
}

static ast_data_type_node *accept_type_declaration() {
    if (!is_type_declaration())
        return NULL;

    consume();
    ast_data_type_node *p = create_ast_data_type_node(accepted(), NULL);
    return p;
}

static ast_data_type_node *expect_type_declaration() {
    if (!is_type_declaration()) {
        parsing_error("was expecting type declaration, got \"%s\"", token_type_name(next()->type));
        return NULL;
    }
    return accept_type_declaration();
}

static char *expect_identifier() {
    if (!expect(TOK_IDENTIFIER))
        return NULL;

    return accepted()->value;
}

void parse_statement() {
    if (is_type_declaration()) {
        // it's a declaration of a variable or function declaration or definition
        ast_data_type_node *dt = accept_type_declaration();
        char *name = expect_identifier();
        if (accept(TOK_LPAREN)) {
            // function call or declaration
            // parse function args (expressions with commas)
            expect(TOK_RPAREN);
        }
        else if (accept(TOK_ASSIGNMENT)) {
            // a variable and a value
            parse_expression_using_shunting_yard();
            expect(TOK_END_OF_STATEMENT);
        }
        else if (accept(TOK_END_OF_STATEMENT)) {
            // variable declaration without value
        }
    }
    else if (accept(TOK_IF)) {
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
    else if (accept(TOK_WHILE)) {
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
    else if (accept(TOK_CONTINUE)) {
        // a continue keyword
    }
    else if (accept(TOK_BREAK)) {
        // a break keyword
    }
    else if (accept(TOK_RETURN)) {
        if (accept(TOK_END_OF_STATEMENT)) {
            // return with no value
        } else {
            parse_expression_using_shunting_yard();
            expect(TOK_END_OF_STATEMENT);
        }
    }
    // if (accept(TOK_IDENTIFIER)) {
    //     // can be declaration, function call or assignment
    //     // I think we should hand those to the expression parser?
    // }
    else {
        parsing_error("unexpected token \"%s\"", token_type_name(next()->type));
    }
}

void parse_block() {
    while (!parsing_failed() && !next_is(TOK_BLOCK_END) && !next_is(TOK_EOF)) {
        parse_statement();
    }
}

ast_var_decl_node *parse_function_arguments_list() {
    ast_var_decl_node *head = NULL, *tail = NULL;

    while (!next_is(TOK_RPAREN)) {
        ast_data_type_node *dt = accept_type_declaration();
        char *name = expect_identifier();
        if (dt == NULL || name == NULL)
            return NULL;
        ast_var_decl_node *n = create_ast_var_decl_node(dt, name);

        // add it to the end of the list
        if (head == NULL) {
            head = n;
            tail = n;
        } else {
            tail->next = n;
            n->next = NULL;
        }

        if (!accept(TOK_COMMA))
            break;
    }

    return head;
}

int parse_file_using_recursive_descend() {
    while (!parsing_failed() && !next_is(TOK_EOF)) {
        ast_data_type_node *dt = expect_type_declaration();
        char *name = expect_identifier();
        if (dt == NULL || name == NULL)
            break;

        if (accept(TOK_END_OF_STATEMENT)) {
            // variable declaration without initial value
            ast_var_decl_node *var = create_ast_var_decl_node(dt, name);
            ast_add_var(var);

        } else if (accept(TOK_ASSIGNMENT)) {
            // variable with initial value
            parse_expression_using_shunting_yard();
            expect(TOK_END_OF_STATEMENT);

        } else if (accept(TOK_LPAREN)) {
            // function declaration or definition
            ast_var_decl_node *args_list = parse_function_arguments_list();
            expect(TOK_RPAREN);

            if (accept(TOK_END_OF_STATEMENT)) {
                // just declaration
                ast_func_decl_node *func = create_ast_func_decl_node(dt, name, args_list, NULL);
                ast_add_func(func);

            } else if (accept(TOK_BLOCK_START)) {
                parse_block();
                expect(TOK_BLOCK_END);
                ast_func_decl_node *func = create_ast_func_decl_node(dt, name, args_list, NULL);
                ast_add_func(func);

            }
        } else {
            parsing_error("unexpected token type \"%s\"\n", token_type_name((next())->type));
        }
    }

    return parsing_failed() ? ERROR : SUCCESS;
}

