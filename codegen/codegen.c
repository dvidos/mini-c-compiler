#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../options.h"
#include "../err_handler.h"
#include "../expression.h"
#include "../lexer/token.h"
#include "../statement.h"
#include "../symbol.h"
#include "../ast_node.h"
#include "interm_repr.h"



// we shall need some infrastructure
// reserve and release registers, a way to allocate things for the segments etc.


// ----------------------------------------------------------



#define WHILES_STACK_SIZE  16
static int ifs_counter = 0;
static int whiles_counter = 0;
static int whiles_stack_len;
static int whiles_stack[WHILES_STACK_SIZE];
static int get_next_if_number() {
    return ++ifs_counter;
}
static void begin_while() {
    if (whiles_stack_len == WHILES_STACK_SIZE) {
        error(NULL, 0, "more than %d nested whiles detected, we need more stack!", WHILES_STACK_SIZE);
        return;
    }
    whiles_stack[whiles_stack_len++] = ++whiles_counter;
}
static int curr_while_number() {
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

// --------------------------------------------------------

struct function_code_generation_info {
    func_declaration *decl;
    // a way to track which registers we use?
    // a way to track arguments and local variables, and their relation to BP
} current_function_generated;


void generate_expression_code(expression *expr) {
    // post-order visit, 
    // each operation is told where to store its result,
    // which is used 
    printf("\t(generating expression code)\n");
}

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
            break;

        case ST_IF:
            // generate condition and jumps in the true and false bodies
            generate_expression_code(stmt->expr);
            int if_no = get_next_if_number();
            printf("\tJNE if_%d_true_body\n", if_no); // we need a name for this if...
            printf("\tJMP if_%d_false_body\n", if_no); // we need a name for this if...
            printf("if_%d_true_body:\n", if_no);
            generate_statement_code(stmt->body);
            printf("\tJMP if_%d_end\n", if_no);
            if (stmt->else_body != NULL) {
                printf("if_%d_false_body:\n", if_no);
                generate_statement_code(stmt->else_body);
            }
            printf("if_%d_end:\n", if_no);
            break;

        case ST_WHILE:
            // generate condition and jumps in the end of the loop
            // we need a stack of whiles
            begin_while();
            printf("while_%d_start:\n", curr_while_number());
            generate_expression_code(stmt->expr);
            printf("\t<condition>\n");
            printf("\tJNE while_%d_end\n", curr_while_number());
            generate_statement_code(stmt->body);
            printf("while_%d_end:\n", curr_while_number());
            end_while();
            break;

        case ST_CONTINUE:
            printf("\tJMP while_%d_start\n", curr_while_number());
            break;

        case ST_BREAK:
            printf("\tJMP while_%d_end\n", curr_while_number());
            break;

        case ST_RETURN:
            if (stmt->expr != NULL) {
                generate_expression_code(stmt->expr);
            }
            printf("\tJMP %s_exit\n", current_function_generated.decl->func_name);
            break;

        case ST_EXPRESSION:
            // generate expression using a DAG? as is?
            printf("\t<expression generation>\n");
            break;    
    }
}

void generate_function_code(func_declaration *func) {
    memset(&current_function_generated, 0, sizeof(struct function_code_generation_info));
    current_function_generated.decl = func;

    // stack frame, decoding of arguments, local data, etc.
    // where do we go from here?

    printf("%s:\n", func->func_name);
    printf("\tPUSH bp\n");
    printf("\tMOV bp, sp\n");

    // allocate stack space for func arguments and locals
    // find all var declarations in the subtree

    statement *stmt = func->stmts_list;
    while (stmt != NULL) {
        generate_statement_code(stmt);
        stmt = stmt->next;
    }

    printf("%s_exit:\n", func->func_name);
    printf("\tPOP bp\n");
    printf("\tRET\n");
    printf("\n");


}

void generate_module_code(ast_module_node *module) {
    statement *stmt = module->statements_list;
    while (stmt != NULL) {
        if (stmt->stmt_type != ST_VAR_DECL) {
            error(NULL, 0, "only var declarations are supported in code generation");
        } else {
            // find size, allocate memory, get address (?)
        }
        stmt = stmt->next;
    }

    func_declaration *func = module->funcs_list;
    while (func != NULL) {
        if (func->stmts_list != NULL)
            generate_function_code(func);
        func = func->next;
    }

    if (options.verbose) {
        printf("--- Generated code follows: ---\n");
        printf(".bss (uninitialized\n");
        ir_dump_data_segment(false);
        printf(".data (initialized\n");
        ir_dump_data_segment(true);
        printf(".text\n");
        ir_dump_code_segment();
        printf("--- end ---\n");
    }
}

