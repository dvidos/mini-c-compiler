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
#include "../declaration.h"
#include "codegen.h"
#include "ir_listing.h"



static void generate_global_vars_code(code_gen *cg, var_declaration *decl, expression *init_expr) {
    int length = length = decl->data_type->ops->size_of(decl->data_type);
    void *init_value = NULL;

    if (init_expr != NULL) {
        if (init_expr->op == OP_STR_LITERAL) {
            length = strlen(init_expr->value.str) + 1;
            init_value = init_expr->value.str;
        } else if (init_expr->op == OP_NUM_LITERAL) {
            init_value = &init_expr->value.num;
        } else if (init_expr->op == OP_CHR_LITERAL) {
            init_value = &init_expr->value.chr;
        } else if (init_expr->op == OP_BOOL_LITERAL) {
            init_value = &init_expr->value.bln;
        } else {
            error(init_expr->token->filename, init_expr->token->line_no,
                "sorry, only literal initial values supported for now");
            return;
        }
    }

    cg->ir->ops->add(cg->ir, new_ir_data_declaration(length, init_value, decl->var_name, IR_GLOBAL, 0));
}


void code_gen_generate_for_module(code_gen *cg, ast_module_node *module) {

    statement *stmt = module->statements_list;
    while (stmt != NULL) {
        if (stmt->stmt_type != ST_VAR_DECL) {
            error(stmt->token->filename, stmt->token->line_no, "only var declarations are supported in code generation");
            return;
        }

        generate_global_vars_code(cg, stmt->decl, stmt->expr);
        stmt = stmt->next;
    }

    func_declaration *func = module->funcs_list;
    while (func != NULL) {
        if (func->stmts_list == NULL) { // it's just a declaration
            func = func->next;
            continue;
        }

        cg->ops->generate_for_function(cg, func);
        func = func->next;
    }
}

