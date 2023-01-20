#include <stddef.h>
#include "../token.h"
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

// -----------------------------

// // we need to have operator precedence for these
// // we also need a "SENTINEL", the lowest precedence of all.
// typedef enum oper {
//     OP_LITERAL,
//     OP_UNARY_MINUS,
//     OP_POINTER_REF,
//     OP_ADD,
//     OP_SUB,
//     OP_MUL,
//     OP_DIV,
//     OP_SENTINEL
// } oper;

// typedef struct {
//     oper oper;
//     expr *arg1; // sole argument for unary expressions
//     expr *arg2;
// } expr;

// // creates an expression with one or two arguments
// static expr *create_expr(oper oper, expr *arg1, expr *arg2) {
//     expr *e = malloc(sizeof(expr));
//     e->oper = oper;
//     e->arg1 = arg1;
//     e->arg2 = arg2;
//     return e;
// }

// // --------------------------------------


// // expression stack for the shunting yard algorithm
// #define EXPR_STACK_SIZE 64
// static expr *expr_stack[EXPR_STACK_SIZE];
// static int expr_stack_len = 0;

// static inline void reset_expr_stack() {
//     expr_stack_len = 0;
// }

// static inline void push_expr(expr *e) {
//     if (expr_stack_len >= EXPR_STACK_SIZE) fail("Expr push: stack full");
//     expr_stack[expr_stack_len++] = e;
// }

// static inline expr *pop_expr() {
//     if (expr_stack_len == 0) fail("Expr pop: stack empty");
//     expr_stack[--expr_stack_len];
// }

// static inline expr *peek_expr() {
//     return (expr_stack_len == 0) ? 0 : expr_stack[expr_stack_len - 1];
// }

// static inline bool expr_stack_empty() {
//     return (expr_stack_len == 0);
// }

// // ------------------------------

// #define OPER_STACK_SIZE 64
// static oper oper_stack[OPER_STACK_SIZE];
// static int oper_stack_len = 0;

// static inline void reset_oper_stack() {
//     oper_stack_len = 0;
// }

// static inline void push_oper(oper open) {
//     if (oper_stack_len >= OPER_STACK_SIZE) fail("oper push: stack full");
//     oper_stack[oper_stack_len++] = open;
// }

// static inline oper pop_oper() {
//     if (oper_stack_len == 0) fail("oper pop: stack empty");
//     oper_stack[--oper_stack_len];
// }

// static inline oper peek_oper() {
//     return (oper_stack_len == 0) ? 0 : oper_stack[oper_stack_len - 1];
// }

// static inline bool oper_stack_empty() {
//     return (oper_stack_len == 0);
// }

// // ------------------------------------

// // attempt at creating a strongly typed container via template
// #define STACK_MAX_SIZE   100
// struct stack {
//     size_t items[STACK_MAX_SIZE];
//     int len;
// };
// #define strongly_typed_stack_template(type)  \
//     static inline void type##_stack_push(type value) { stack_push((size_t)value); }  \
//     static inline type type##_stack_pop() { (type)stack_pop(); }  \
//     static inline type type##_stack_peek() { (type)stack_peek(); }

// typedef expr *expr_ptr;
// strongly_typed_stack_template(expr_ptr);
// strongly_typed_stack_template(oper);

// void a() {
//     expr_ptr_stack_push(NULL);
// }

// // ------------------------------------------

// static oper make_binary_operator(token *t) {
//     switch (t->type) {
//         case TOK_PLUS_SIGN: return OP_ADD;
//         case TOK_MINUS_SIGN: return OP_SUB;
//         case TOK_STAR: return OP_MUL;
//         case TOK_SLASH: return OP_DIV;
//     }
// }

// static oper make_unary_operator(token *t) {
//     switch (t->type) {
//         case TOK_MINUS_SIGN: return OP_UNARY_MINUS;
//         case TOK_STAR: return OP_POINTER_REF;
//     }
// }

// static inline expr *make_literal(token *t) {
//     return create_expr(OP_LITERAL, t->value, NULL);
// }

// static inline expr *make_tree(oper oper, expr *arg1, expr *arg2) {
//     return create_expr(oper, arg1, arg2);
// }

// // ---------------------------------------------------

// void parse_expression_using_yard_shunting_algorithm(token *first_token) {
//     reset_expr_stack();
//     reset_oper_stack();
//     push_oper(OP_SENTINEL);
//     parse_expr();
//     expect(end_or_rparen);
// }

// void parse_expr() {
//     parse_part();
//     while (next is binary operator) {
//         push_operator(make_binary_operator(t))
//         consume()
//         parse_part()
//     }
//     while (peek_oper() != OP_SENTINEL) {
//         pop_operator()
//     }
// }

// void parse_part() {
//     if (next is a literal) {
//         push_expr(make_literal(next));
//         consume
//     } else if (next is lparen) {
//         consume
//         push_oper(OP_SENTINEL)
//         parse_expr();
//         expect(rparen)
//         pop_oper() // remove the sentinel
//     } else if (next is unary operator) {
//         push_oper(make_unary_operator(next));
//         consume
//         parse_part()
//     } else {
//         error()
//     }
// }

// void pop_operator() {

// }

// void push_operator() {

// }

ast_expression_node *parse_expression_using_shunting_yard() {
    // skipping for now
    while (!next_is(TOK_END_OF_STATEMENT) && !next_is(TOK_RPAREN))
        consume();

    return NULL;
}

