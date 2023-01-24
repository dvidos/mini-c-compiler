#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "../defs.h"
#include "../lexer/token.h"
#include "../ast_node.h"
#include "../ast.h"
#include "iterator.h"
#include "shunting_yard.h"


// light & simple list implementation for functions
#define declare_list(type)   type *list = NULL, *list_tail = NULL
#define list_append(var)     if (list_tail == NULL) { list = var; list_tail = var; } \
                             else { list_tail->next = var; list_tail = var; }


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

static char *expect_identifier();
static bool is_data_type_description();
static bool is_variable_declaration();
static bool is_function_declaration();

static data_type *accept_data_type_description();
static ast_statement_node *accept_variable_declaration();
static ast_func_decl_node *accept_function_declaration();

static ast_statement_node *parse_statement();
static ast_statement_node *parse_block(); // parses statements in a loop
static ast_var_decl_node *parse_function_arguments_list();

static bool is_data_type_description() {
    // storage_class_specifiers: typedef, extern, static, auto, register.
    // type_qualifiers: const, volatile.
    // type specifiers: void, char, short, int, long, float, double, 
    //                  signed, unsigned, <struct/union/enum>, <type-name>
    // make sure to detect without consuming anything. 
    // use lookahead() if neded.

    // keeping it simple for now
    return (next_is(TOK_INT_KEYWORD)
         || next_is(TOK_CHAR_KEYWORD)
         || next_is(TOK_VOID));
}

static bool is_variable_declaration() {
    // we assume data definition takes only one token for now
    if (!is_data_type_description())
        return false;

    // after that, we should expect an identifier, but NO parenthesis
    return lookahead_is(1, TOK_IDENTIFIER) && !lookahead_is(2, TOK_LPAREN);
}

static bool is_function_declaration() {

    // we assume data definition takes only one token for now
    if (!is_data_type_description())
        return false;

    // after that, we should expect an identifier and a L parenthesis
    return lookahead_is(1, TOK_IDENTIFIER) && lookahead_is(2, TOK_LPAREN);
}

static char *expect_identifier() {
    if (!expect(TOK_IDENTIFIER))
        return NULL;

    return accepted()->value;
}

static data_type *accept_data_type_description() {
    // we assume data definition takes only one token, for now
    if (!is_data_type_description())
        return NULL;

    // we assume data definition takes only one token, for now
    consume();
    return create_ast_data_type_node(accepted(), NULL);
}

static ast_statement_node *accept_variable_declaration() {
    if (!is_variable_declaration())
        return NULL;

    data_type *dt = accept_data_type_description();
    if (dt == NULL) return NULL;
    char *name = expect_identifier();
    if (name == NULL) return NULL;

    ast_var_decl_node *vd = create_ast_var_decl_node(dt, name);
    expr_node *initialization = NULL;
    if (accept(TOK_ASSIGNMENT)) {
        initialization = parse_expression_using_shunting_yard();
    }

    if (!expect(TOK_END_OF_STATEMENT))
        return NULL;
    return create_ast_decl_statement(vd, initialization);
}

static ast_func_decl_node *accept_function_declaration() {
    if (!is_function_declaration())
        return NULL;

    data_type *ret_type = accept_data_type_description();
    if (ret_type == NULL) return NULL;

    char *name = expect_identifier();
    if (name == NULL) return NULL;

    if (!expect(TOK_LPAREN)) return NULL;
    ast_var_decl_node *args = NULL;
    if (!accept(TOK_RPAREN)) {
        args = parse_function_arguments_list();
        if (!expect(TOK_RPAREN)) return NULL;
    }

    ast_statement_node *body = NULL;
    if (!accept(TOK_END_OF_STATEMENT)) {
        body = parse_statement();
    }

    // no ";" required after functions

    return create_ast_func_decl_node(ret_type, name, args, body);
}

// cannot parse a function, but can parse a block and anything in it.
static ast_statement_node *parse_statement() {

    if (accept(TOK_BLOCK_START)) {
        // we need to parse the nested block
        ast_statement_node *bl = parse_block();
        if (!expect(TOK_BLOCK_END)) return NULL;
        return bl;
    }

    if (is_variable_declaration()) {
        return accept_variable_declaration();
    }

    if (accept(TOK_IF)) {
        if (!expect(TOK_LPAREN)) return NULL;
        expr_node *cond = parse_expression_using_shunting_yard();
        if (!expect(TOK_RPAREN)) return NULL;
        ast_statement_node *if_body = parse_statement();
        if (if_body == NULL) return NULL;
        ast_statement_node *else_body = NULL;
        if (accept(TOK_ELSE)) {
            else_body = parse_statement();
            if (else_body == NULL) return NULL;
        }
        return create_ast_if_statement(cond, if_body, else_body);
    }

    if (accept(TOK_WHILE)) {
        if (!expect(TOK_LPAREN)) return NULL;
        expr_node *cond = parse_expression_using_shunting_yard();
        if (!expect(TOK_RPAREN)) return NULL;
        ast_statement_node *body = parse_statement();
        if (body == NULL) return NULL;
        return create_ast_while_statement(cond, body);
    }

    if (accept(TOK_CONTINUE)) {
        if (!expect(TOK_END_OF_STATEMENT)) return NULL;
        return create_ast_continue_statement();
    }

    if (accept(TOK_BREAK)) {
        if (!expect(TOK_END_OF_STATEMENT)) return NULL;
        return create_ast_break_statement();
    }

    if (accept(TOK_RETURN)) {
        expr_node *value = NULL;
        if (!accept(TOK_END_OF_STATEMENT)) {
            value = parse_expression_using_shunting_yard();
            if (!expect(TOK_END_OF_STATEMENT)) return NULL;
        }
        return create_ast_return_statement(value);
    }
    
    // what is left?
    expr_node *expr = parse_expression_using_shunting_yard();
    if (!expect(TOK_END_OF_STATEMENT)) return NULL;
    return create_ast_expr_statement(expr);
}

static ast_statement_node *parse_block() {
    declare_list(ast_statement_node);

    while (!parsing_failed() && !next_is(TOK_BLOCK_END) && !next_is(TOK_EOF)) {
        ast_statement_node *n = parse_statement();
        if (n == NULL) // error?
            return NULL;
        list_append(n);
    }

    return create_ast_block_node(list);
}

static ast_var_decl_node *parse_function_arguments_list() {
    declare_list(ast_var_decl_node);

    while (!next_is(TOK_RPAREN)) {
        data_type *dt = accept_data_type_description();
        char *name = expect_identifier();
        if (dt == NULL || name == NULL)
            return NULL;
        ast_var_decl_node *n = create_ast_var_decl_node(dt, name);
        list_append(n);

        if (!accept(TOK_COMMA))
            break;
    }

    return list;
}


static void parse_file_level_element() {
    if (is_variable_declaration()) {
        ast_statement_node *n = accept_variable_declaration();
        ast_add_statement(n);
    }
    else if (is_function_declaration()) {
        ast_func_decl_node *n = accept_function_declaration();
        ast_add_function(n);
    }
    else {
        parsing_error("expecting variable or function declaration");
    }
}

int parse_file_using_recursive_descend() {
    while (!parsing_failed() && !next_is(TOK_EOF))
        parse_file_level_element();
    
    return parsing_failed() ? ERROR : SUCCESS;
}
