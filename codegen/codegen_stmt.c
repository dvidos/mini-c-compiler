#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../options.h"
#include "../err_handler.h"
#include "../expression.h"
#include "../lexer/token.h"
#include "../statement.h"
#include "../symbol.h"
#include "../ast_node.h"
#include "interm_repr.h"
#include "codegen.h"


static int get_next_if_num();
static void begin_while();
static int get_curr_while_num();
static void end_while();


void generate_statement_code(statement *stmt) {
    char label[32];

    // blocks of statements and expressions, together with jumps
    switch (stmt->stmt_type) {
        case ST_BLOCK:
            // don't we need to push scope? in order
            // to make sure inner names resolve to correct target?
            statement *entry = stmt->body;
            while (entry != NULL) {
                generate_statement_code(entry);
                entry = entry->next;
            }
            break;

        case ST_VAR_DECL:
            // maybe assignment of value?
            // in theory, stack allocation has already happened
            if (stmt->expr) {
                // must emulate an assignment expression.
                generate_expression_code(stmt->expr, GT_GENERIC);
            }
            break;

        case ST_IF:
            // generate condition and jumps in the true and false bodies
            int ifnum = get_next_if_num();
            if (stmt->else_body == NULL) {
                // simple if
                generate_expression_code(stmt->expr, GT_BRANCH_DECISION);
                ir_jmp_if(false, "if%d_end", ifnum);
                generate_statement_code(stmt->body);
                ir_set_next_label("if%d_end", ifnum);
            } else {
                // if & else
                generate_expression_code(stmt->expr, GT_BRANCH_DECISION);
                ir_jmp_if(false, "if%d_false", ifnum);
                generate_statement_code(stmt->body);
                ir_jmp("if%d_end", ifnum);
                ir_set_next_label("if%d_false", ifnum);
                generate_statement_code(stmt->else_body);
                ir_set_next_label("if%d_end", ifnum);
            }
            break;

        case ST_WHILE:
            // generate condition and jumps in the end of the loop
            // we need a stack of whiles
            begin_while();
            ir_set_next_label("wh%d", get_curr_while_num());
            generate_expression_code(stmt->expr, GT_BRANCH_DECISION);
            ir_jmp_if(false, "wh%d_end", get_curr_while_num());
            generate_statement_code(stmt->body);
            ir_set_next_label("wh%d_end", get_curr_while_num());
            end_while();
            break;

        case ST_CONTINUE:
            ir_add_str("JMP wh%d", get_curr_while_num());
            break;

        case ST_BREAK:
            ir_add_str("JMP wh%d_end", get_curr_while_num());
            break;

        case ST_RETURN:
            if (stmt->expr != NULL) {
                generate_expression_code(stmt->expr, GT_RETURNED_VALUE);
            }
            ir_jmp("%s_exit", current_function_generated.decl->func_name);
            break;

        case ST_EXPRESSION:
            // generate expression using a DAG? as is?
            generate_expression_code(stmt->expr, GT_GENERIC);
            break;    
    }
}

// --------------------------------------------------------------------

#define WHILES_STACK_SIZE  16

static int ifs_counter = 0;
static int whiles_counter = 0;
static int whiles_stack_len;
static int whiles_stack[WHILES_STACK_SIZE];

static int get_next_if_num() {
    return ++ifs_counter;
}

static void begin_while() {
    if (whiles_stack_len == WHILES_STACK_SIZE) {
        error(NULL, 0, "more than %d nested whiles detected, we need more stack!", WHILES_STACK_SIZE);
        return;
    }
    whiles_stack[whiles_stack_len++] = ++whiles_counter;
}

static int get_curr_while_num() {
    if (whiles_stack_len == 0) {
        error(NULL, 0, "while was expected, but none started");
        return 0;
    }
    return whiles_stack[whiles_stack_len - 1];
}

static void end_while() {
    if (whiles_stack_len == 0) {
        error(NULL, 0, "stack of whiles underflow!");
        return;
    }
    whiles_stack_len--;
}

