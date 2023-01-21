#include <stddef.h>
#include <stdlib.h>
#include "../token.h"
#include "../operators.h"
#include "../ast_node.h"
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
        || tt == TOK_IDENTIFIER;
}

ast_expression_node *accept_terminal() {
    if (!next_is_terminal()) return NULL;
    token *t = next();
    consume();
    switch (t->type) {
        case TOK_IDENTIFIER: return create_ast_expr_name(t->value);
        case TOK_STRING_LITERAL: return create_ast_expr_string_literal(t->value);
        case TOK_NUMERIC_LITERAL: return create_ast_expr_numeric_literal(atol(t->value));
        case TOK_CHAR_LITERAL: return create_ast_expr_char_literal(t->value[0]);
    }
    return NULL;
}

// -------------------------------------------------------------------




// -------------------------------------------------------------------

#define MAX_STACK_SIZE 64
struct stack {
    void *values[MAX_STACK_SIZE];
    int length;
};
void   reset_stack(struct stack *s)       { s->length = 0; }
bool   stack_empty(struct stack *s)       { return s->length == 0; }
void   push(struct stack *s, void *value) { s->values[s->length++] = value; }
void *pop(struct stack *s)                { return s->values[--s->length]; }
void *peek(struct stack *s)               { return s->values[s->length - 1]; }
struct stack operators_stack, *operators = &operators_stack; 
struct stack operands_stack, *operands = &operands_stack;

// -------------------------------------------------------------------

static void parse_complex_expression();
static void parse_operand();
static void push_operator(oper op);
static void pop_operator();

static void parse_complex_expression() {
    parse_operand();
    while (next_is_binary_operator()) {
        push_operator(accept_binary_operator());
        parse_operand();
    }
    while ((oper)peek(operators) != OP_SENTINEL)
        pop_operator();
}

static void parse_operand() {
    if (next_is_terminal()) {
        push(operands, accept_terminal());
    }
    else if (accept(TOK_LPAREN)) {
        push(operators, (void *)OP_SENTINEL);
        parse_complex_expression();
        expect(TOK_RPAREN);
        pop(operators); // pop sentinel
    }
    else if (next_is_unary_operator()) {
        oper op = accept_unary_operator();
        push_operator(op);
        parse_operand();
    } else {
        parsing_error("expected '(', unary operator, or terminal token");
    }
}

static void push_operator(oper op) {
    // if a higher priority operator exists in the stack,
    // we must extract it to an expression now,
    // to force it to be calculated first
    while (oper_precedence((oper)peek(operators)) > oper_precedence(op))
        pop_operator();
    
    // we are equal or higher priority than the things on the stack,
    // so popping will naturally be in priority order
    push(operators, (void *)op);
}

// extract from stack and combine into new expressions
static void pop_operator() {
    bool top_is_binary = to_binary_operator((oper)peek(operators)) != OP_UNKNOWN;
    ast_expression_node *op1, *op2;
    oper op;

    if (top_is_binary) { // pop two operands and combine
        op1 = pop(operands);
        op2 = pop(operands);
        op = (oper)pop(operators);
        push(operands, create_ast_expression(op, op1, op2));
    } else {
        // top operator is unary, pop only one operand
        op1 = pop(operands);
        op = (oper)pop(operators);
        push(operands, create_ast_expression(op, op1, NULL));
    }
}

// -------------------------------------------------------------------

ast_expression_node *parse_expression_using_shunting_yard() {

    reset_stack(operators);
    reset_stack(operands);
    push(operators, (void *)OP_SENTINEL);
    parse_complex_expression();

    return (ast_expression_node *)pop(operands);
}
