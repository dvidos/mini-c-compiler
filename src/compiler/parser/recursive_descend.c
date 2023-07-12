#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "../../err_handler.h"
#include "../lexer/token.h"
#include "../declaration.h"
#include "../statement.h"
#include "../ast.h"
#include "token_iterator.h"
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

static const char *expect_identifier(token_iterator *ti);
static bool is_data_type_description(token_iterator *ti, int *num_tokens);
static bool is_variable_declaration(token_iterator *ti);
static bool is_function_declaration(token_iterator *ti);

static data_type *accept_data_type_description(mempool *mp, token_iterator *ti);
static statement *accept_variable_declaration(mempool *mp, token_iterator *ti);
static func_declaration *accept_function_declaration(mempool *mp, token_iterator *ti);

static statement *parse_statement(mempool *mp, token_iterator *ti);
static statement *parse_statements_list_in_block(mempool *mp, token_iterator *ti);
static var_declaration *parse_function_arguments_list(mempool *mp, token_iterator *ti);

static bool is_data_type_description(token_iterator *ti, int *num_tokens) {

    // storage_class_specifiers: typedef, extern, static, auto, register.
    // type_qualifiers: const, volatile.
    // type specifiers: void, char, short, int, long, float, double, 
    //                  signed, unsigned, <struct/union/enum>, <type-name>
    // make sure to detect without consuming anything. 
    // use lookahead() if neded.

    int count = 0;

    if (!  (ti->next_is(ti, TOK_INT_KEYWORD)
         || ti->next_is(ti, TOK_FLOAT)
         || ti->next_is(ti, TOK_CHAR_KEYWORD)
         || ti->next_is(ti, TOK_BOOL)
         || ti->next_is(ti, TOK_VOID))) {
        return false;
    }
    count++;

    // possible pointer
    if (ti->lookahead_is(ti, count, TOK_STAR))
        count++;
    
    // possible pointer-to-pointer
    if (ti->lookahead_is(ti, count, TOK_STAR))
        count++;

    *num_tokens = count;
    return true;
}

static bool is_variable_declaration(token_iterator *ti) {
    // we assume data definition takes only one token for now
    int num_tokens = 0;
    if (!is_data_type_description(ti, &num_tokens))
        return false;

    // after that, we should expect an identifier, but NO parentheses
    return ti->lookahead_is(ti, num_tokens + 0, TOK_IDENTIFIER)
        && !ti->lookahead_is(ti, num_tokens + 1, TOK_LPAREN);
}

static bool is_function_declaration(token_iterator *ti) {

    // we assume data definition takes only one token for now
    int num_tokens = 0;
    if (!is_data_type_description(ti, &num_tokens))
        return false;

    // after that, we should expect an identifier and a L parenthesis
    return ti->lookahead_is(ti, num_tokens + 0, TOK_IDENTIFIER) 
        && ti->lookahead_is(ti, num_tokens + 1, TOK_LPAREN);
}

static const char *expect_identifier(token_iterator *ti) {
    if (!ti->expect(ti, TOK_IDENTIFIER))
        return NULL;

    return ti->accepted(ti)->value;
}

static data_type *accept_data_type_description(mempool *mp, token_iterator *ti) {
    // we assume data definition takes only one token, for now
    int tokens;
    if (!is_data_type_description(ti, &tokens))
        return NULL;

    ti->consume(ti); // a keyword such as "int" or "char"
    type_family family = data_type_family_for_token(ti->accepted(ti)->type);
    data_type *t = new_data_type(family, NULL);

    if (ti->accept(ti, TOK_STAR)) {
        // we are a pointer, nest the data type
        t = new_data_type(TF_POINTER, t);
    }

    if (ti->accept(ti, TOK_STAR)) {
        // we are a pointer to pointer, nest the data type too
        t = new_data_type(TF_POINTER, t);
    }

    return t;
}

static statement *accept_variable_declaration(mempool *mp, token_iterator *ti) {
    if (!is_variable_declaration(ti))
        return NULL;

    data_type *dt = accept_data_type_description(mp, ti);
    if (dt == NULL) return NULL;
    const char *name = expect_identifier(ti);
    token *identifier_token = ti->accepted(ti);
    if (name == NULL) return NULL;

    if (ti->accept(ti, TOK_LBRACKET)) {
        // it's an array
        dt = new_data_type(TF_ARRAY, dt);
        if (!ti->expect(ti, TOK_NUMERIC_LITERAL)) return NULL;
        dt->array_size = strtol(ti->accepted(ti)->value, NULL, 10);
        if (!ti->expect(ti, TOK_RBRACKET)) return NULL;

        if (ti->accept(ti, TOK_LBRACKET)) {
            // it's a two-dimensions array
            dt = new_data_type(TF_ARRAY, dt);
            if (!ti->expect(ti, TOK_NUMERIC_LITERAL)) return NULL;
            dt->array_size = strtol(ti->accepted(ti)->value, NULL, 10);
            if (!ti->expect(ti, TOK_RBRACKET)) return NULL;
        }
    }

    var_declaration *vd = new_var_declaration(dt, name, identifier_token);
    expression *initialization = NULL;
    if (ti->accept(ti, TOK_EQUAL_SIGN)) {
        initialization = parse_expression_using_shunting_yard(mp, ti);
    }

    if (!ti->expect(ti, TOK_SEMICOLON))
        return NULL;
    return new_var_decl_statement(vd, initialization, identifier_token);
}

static func_declaration *accept_function_declaration(mempool *mp, token_iterator *ti) {
    if (!is_function_declaration(ti))
        return NULL;

    data_type *ret_type = accept_data_type_description(mp, ti);
    if (ret_type == NULL) return NULL;

    const char *name = expect_identifier(ti);
    if (name == NULL) return NULL;
    token *identifier_token = ti->accepted(ti);

    if (!ti->expect(ti, TOK_LPAREN)) return NULL;
    var_declaration *args = NULL;
    if (!ti->accept(ti, TOK_RPAREN)) {
        args = parse_function_arguments_list(mp, ti);
        if (!ti->expect(ti, TOK_RPAREN)) return NULL;
    }

    statement *body = NULL;
    // we either have a semicolon (declaration) or an opening brace (definition)
    if (ti->accept(ti, TOK_SEMICOLON)) {
        body = NULL;
    } else if (ti->accept(ti, TOK_BLOCK_START)) {
        body = parse_statements_list_in_block(mp, ti);
        ti->expect(ti, TOK_BLOCK_END);
    } else {
        error_at(ti->next(ti)->filename, ti->next(ti)->line_no,
            "expecting either ';' or '{' for function %s", name);
    }

    return new_func_declaration(ret_type, name, args, body, identifier_token);
}

// cannot parse a function, but can parse a block and anything in it.
static statement *parse_statement(mempool *mp, token_iterator *ti) {
    token *start_token;

    if (ti->accept(ti, TOK_BLOCK_START)) {
        // we need to parse the nested block, blocks have their own scope
        token *opening_token = ti->accepted(ti);
        statement *stmt_list = parse_statements_list_in_block(mp, ti);
        statement *bl = new_statements_block(stmt_list, opening_token);
        if (!ti->expect(ti, TOK_BLOCK_END)) return NULL;
        return bl;
    }

    if (is_variable_declaration(ti)) {
        return accept_variable_declaration(mp, ti);
    }

    if (ti->accept(ti, TOK_IF)) {
        start_token = ti->accepted(ti);
        if (!ti->expect(ti, TOK_LPAREN)) return NULL;
        expression *cond = parse_expression_using_shunting_yard(mp, ti);
        if (!ti->expect(ti, TOK_RPAREN)) return NULL;
        statement *if_body = parse_statement(mp, ti);
        if (if_body == NULL) return NULL;
        statement *else_body = NULL;
        if (ti->accept(ti, TOK_ELSE)) {
            else_body = parse_statement(mp, ti);
            if (else_body == NULL) return NULL;
        }
        return new_if_statement(cond, if_body, else_body, start_token);
    }

    if (ti->accept(ti, TOK_WHILE)) {
        start_token = ti->accepted(ti);
        if (!ti->expect(ti, TOK_LPAREN)) return NULL;
        expression *cond = parse_expression_using_shunting_yard(mp, ti);
        if (!ti->expect(ti, TOK_RPAREN)) return NULL;
        statement *body = parse_statement(mp, ti);
        if (body == NULL) return NULL;
        return new_while_statement(cond, body, start_token);
    }

    if (ti->accept(ti, TOK_CONTINUE)) {
        start_token = ti->accepted(ti);
        if (!ti->expect(ti, TOK_SEMICOLON)) return NULL;
        return create_continue_statement(start_token);
    }

    if (ti->accept(ti, TOK_BREAK)) {
        start_token = ti->accepted(ti);
        if (!ti->expect(ti, TOK_SEMICOLON)) return NULL;
        return new_break_statement(start_token);
    }

    if (ti->accept(ti, TOK_RETURN)) {
        start_token = ti->accepted(ti);
        expression *value = NULL;
        if (!ti->accept(ti, TOK_SEMICOLON)) {
            value = parse_expression_using_shunting_yard(mp, ti);
            if (!ti->expect(ti, TOK_SEMICOLON)) return NULL;
        }
        return new_return_statement(value, start_token);
    }
    
    // what is left? treat the rest as expressions
    start_token = ti->next(ti);
    expression *expr = parse_expression_using_shunting_yard(mp, ti);
    if (!ti->expect(ti, TOK_SEMICOLON)) return NULL;
    return new_expr_statement(expr, start_token);
}

static statement *parse_statements_list_in_block(mempool *mp, token_iterator *ti) {
    declare_list(statement);

    while (!ti->next_is(ti, TOK_BLOCK_END) && !ti->next_is(ti, TOK_EOF) && errors_count == 0) {
        statement *n = parse_statement(mp, ti);
        if (n == NULL) // error?
            return NULL;
        list_append(n);
    }

    return list;
}

static var_declaration *parse_function_arguments_list(mempool *mp, token_iterator *ti) {
    declare_list(var_declaration);

    while (!ti->next_is(ti, TOK_RPAREN)) {
        data_type *dt = accept_data_type_description(mp, ti);
        if (dt == NULL) return NULL;
        const char *name = expect_identifier(ti);
        if (name == NULL) return NULL;
        token *identifier_token = ti->accepted(ti);

        // it's an array
        if (ti->accept(ti, TOK_LBRACKET)) {
            dt = new_data_type(TF_ARRAY, dt);
            if (!ti->expect(ti, TOK_NUMERIC_LITERAL)) return NULL;
            dt->array_size = strtol(ti->accepted(ti)->value, NULL, 10);
            if (!ti->expect(ti, TOK_RBRACKET)) return NULL;

            if (ti->accept(ti, TOK_LBRACKET)) {
                // it's a two-dimensions array
                dt = new_data_type(TF_ARRAY, dt);
                if (!ti->expect(ti, TOK_NUMERIC_LITERAL)) return NULL;
                dt->array_size = strtol(ti->accepted(ti)->value, NULL, 10);
                if (!ti->expect(ti, TOK_RBRACKET)) return NULL;
            }
        }

        var_declaration *n = new_var_declaration(dt, name, identifier_token);
        list_append(n);

        if (!ti->accept(ti, TOK_COMMA))
            break;
    }

    return list;
}

static void parse_file_level_element(mempool *mp, token_iterator *ti) {
    if (is_variable_declaration(ti)) {
        statement *n = accept_variable_declaration(mp, ti);
        ast_add_statement(n);
    }
    else if (is_function_declaration(ti)) {
        func_declaration *n = accept_function_declaration(mp, ti);
        ast_add_function(n);
    }
    else {
        error_at(ti->next(ti)->filename, ti->next(ti)->line_no,
            "expecting variable or function declaration");
    }
}

ast_module_node *parse_file_tokens_using_recursive_descend(mempool *mp, llist *tokens) {

    init_operators(); // make sure our lookup is populated

    // use the new token_iterator.
    token_iterator *ti = new_token_iterator(mp, tokens);
    while (!ti->next_is(ti, TOK_EOF) && errors_count == 0)
        parse_file_level_element(mp, ti);
}

void parse_file_using_recursive_descend___deprecated() {
    // // parsing cannot continue if errors are discovered
    // while (!next_is(TOK_EOF) && errors_count == 0)
    //     parse_file_level_element();
}
