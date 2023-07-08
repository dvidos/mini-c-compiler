#include <stddef.h>
#include <stdlib.h>
#include "../../err_handler.h"
#include "../lexer/token.h"
#include "../operators.h"
#include "../declaration.h"
#include "../expression.h"
#include "iterator.h"

/*
    for the shunting yard algorithm, for parsing operators according to precedence,
    https://www.engr.mun.ca/~theo/Misc/exp_parsing.htm#shunting_yard
    we shall need:
    - a stack of expressions, each being either a terminal, or a subexpression (a binary node)
    - a stack of operators, each with their precedence (an empty stack has the lowest priority)

    we make a "tree" by popping two operands and one operator from their stacks,
    create a node that points to them and push that on the operand stack.

    we do this if we finished, or if the operator we are working with
    has lower precedence by the operator at the top of the stack.
*/

// whether the next token in the iterator can be used as a unary operator
bool next_is_unary_operator() {
    token_type tt = next() == NULL ? TOK_EOF : next()->type;
    return to_unary_operator(tt) != OP_UNKNOWN;
}

oper accept_unary_operator() {
    if (!next_is_unary_operator())
        return OP_UNKNOWN;
    token *t = next();
    consume();
    return to_unary_operator(t->type);
}

bool next_is_postfix_operator() {
    token_type tt = next() == NULL ? TOK_EOF : next()->type;
    return to_postfix_operator(tt) != OP_UNKNOWN;
}

oper accept_postfix_operator() {
    if (!next_is_postfix_operator())
        return OP_UNKNOWN;
    token *t = next();
    consume();
    return to_postfix_operator(t->type);
}

bool next_is_binary_operator() {
    token_type tt = next() == NULL ? TOK_EOF : next()->type;
    return to_binary_operator(tt) != OP_UNKNOWN;
}

oper accept_binary_operator() {
    if (!next_is_binary_operator())
        return OP_UNKNOWN;
    token *t = next();
    consume();
    return to_binary_operator(t->type);
}

bool next_is_terminal() {
    token_type tt = next() == NULL ? TOK_EOF : next()->type;
    return tt == TOK_STRING_LITERAL
        || tt == TOK_NUMERIC_LITERAL 
        || tt == TOK_CHAR_LITERAL 
        || tt == TOK_TRUE
        || tt == TOK_FALSE
        || tt == TOK_IDENTIFIER;
}

expression *accept_terminal() {
    if (!next_is_terminal())
        return NULL;
    token *t = next();
    consume();
    switch (t->type)
    {
        case TOK_IDENTIFIER:      return create_symbol_name_expr(t->value, t);
        case TOK_STRING_LITERAL:  return create_string_literal_expr(t->value, t);
        case TOK_NUMERIC_LITERAL: return create_number_literal_expr(t->value, t);
        case TOK_CHAR_LITERAL:    return create_char_literal_expr(t->value[0], t);
        case TOK_TRUE:            return create_bool_literal_expr(true, t);
        case TOK_FALSE:           return create_bool_literal_expr(false, t);
    }
    return NULL;
}

// -------------------------------------------------------------------

// stacks being strongly typed helps with watches in debugging

#define MAX_STACK_SIZE 64

oper operators_stack[MAX_STACK_SIZE];
int operators_stack_len = 0;
static inline void push_operator(oper op) { operators_stack[operators_stack_len++] = op; if (operators_stack_len >= MAX_STACK_SIZE) error("op stack overflow"); }
static inline oper pop_operator() { return operators_stack[--operators_stack_len]; }
static inline oper peek_operator() { return operators_stack[operators_stack_len - 1]; }

expression *operands_stack[MAX_STACK_SIZE];
int operands_stack_len = 0;
static inline void push_operand(expression *n) { operands_stack[operands_stack_len++] = n; if (operands_stack_len >= MAX_STACK_SIZE) error("expr stack overflow"); }
static inline expression *pop_operand() { return operands_stack[--operands_stack_len]; }
static inline expression *peek_operand() { return operands_stack[operands_stack_len - 1]; }

// -------------------------------------------------------------------

static void parse_complex_expression();
static void parse_operand();
static void push_operator_with_priority(oper op);
static void pop_operator_into_expression();

static void parse_complex_expression() {

    // staring with an operand (number, symbol etc)
    parse_operand();

    // for as long as there are more operators and operands, continue
    while (next_is_binary_operator()) {
        push_operator_with_priority(accept_binary_operator());
        parse_operand();
    }

    // convert any outstanding operators into expressions
    while (peek_operator() != OP_SENTINEL) {
        pop_operator_into_expression();
    }
}

static void parse_operand() {

    if (next_is_terminal()) {
        push_operand(accept_terminal());

    } else if (accept(TOK_LPAREN)) {
        push_operator(OP_SENTINEL);
        parse_complex_expression();
        expect(TOK_RPAREN);
        pop_operator(); // pop sentinel

    } else if (next_is_unary_operator()) {
        push_operator_with_priority(accept_unary_operator());
        parse_operand();

    } else {
        error_at(next()->filename, next()->line_no, "expected '(', unary operator, or terminal token");
    }

    while (next_is_postfix_operator()) {
        oper op = accept_postfix_operator();
        push_operator_with_priority(op);

        if (op == OP_FUNC_CALL) {
            if (accept(TOK_RPAREN)) {
                push_operand(NULL); // i.e. no arguments for the function call
            } else {
                push_operator(OP_SENTINEL);
                parse_complex_expression();
                pop_operator(); // pop sentinel
                expect(TOK_RPAREN);
            }
        } else if (op == OP_ARRAY_SUBSCRIPT) {
            push_operator(OP_SENTINEL);
            parse_complex_expression();
            pop_operator(); // pop sentinel
            expect(TOK_RBRACKET);
        } else if (!is_unary_operator(op)) {
            // post-increment is not binary, it does not expect another operand
            // otoh, array subscript is binary, so it needs another operand
            // pushing a sentinel forces the subexpression to be parsed
            // without interfering with the current contents of the stacks
            push_operator(OP_SENTINEL);
            parse_complex_expression();
            pop_operator(); // pop sentinel
        }
    }
}

static void push_operator_with_priority(oper op) {

    // if a higher priority operator exists in the stack,
    // we must extract it to a new expression now,
    // to force it to be calculated first
    oper top_operator = peek_operator();
    while (oper_precedence(top_operator) > oper_precedence(op)) {
        pop_operator_into_expression();
        top_operator = peek_operator();
    }

    // we are equal or higher priority than the things on the stack,
    // so popping will naturally be in priority order
    push_operator(op);
}

// extract from stack and combine into new expressions
static void pop_operator_into_expression()
{
    oper op = pop_operator();
    bool is_unary = is_unary_operator(op);
    expression *op1, *op2;
    expression *expr;

    if (is_unary) { 
        op1 = pop_operand();
        expr = create_expression(op, op1, NULL, op1->token);
    } else { 
        op1 = pop_operand();
        op2 = pop_operand();
        // note op2, then op1, to make them appear in "correct" order as a result
        // also, op1 may be null, e.g. when calling a func without args
        token *token = op1 == NULL ? (op2 == NULL ? NULL : op2->token) : op1->token;
        expr = create_expression(op, op2, op1, token); 
    }

    push_operand(expr);
}

// -------------------------------------------------------------------

expression *parse_expression_using_shunting_yard() {

    // reset stacks, push SENTINEL to serve as lowest priority indicator
    operators_stack_len = 0;
    operands_stack_len = 0;
    push_operator(OP_SENTINEL);
    parse_complex_expression();

    // all outstanding operators must have been popped into expressions,
    // with the highest prioririty one pushed last
    return pop_operand();
}
