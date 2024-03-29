#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../../run_info.h"
#include "../../err_handler.h"
#include "../lexer/token.h"
#include "../ast/all.h"
#include "../ast/all.h"
#include "../ast/all.h"
#include "../ast/all.h"
#include "codegen.h"
#include "ir_listing.h"



static void gen_global_var(code_gen *cg, ast_variable *decl, ast_expression *init_expr) {
    int length = decl->data_type->ops->size_of(decl->data_type);
    void *init_value = NULL;
    ir_data_storage storage = IR_GLOBAL;
    
    if (init_expr != NULL) {
        if (init_expr->op == OP_STR_LITERAL) {
            length = strlen(init_expr->value.str) + 1;
            init_value = (void *)init_expr->value.str;
            storage = IR_GLOBAL_RO;
        } else if (init_expr->op == OP_NUM_LITERAL) {
            init_value = &init_expr->value.num;
        } else if (init_expr->op == OP_CHR_LITERAL) {
            init_value = &init_expr->value.chr;
        } else if (init_expr->op == OP_BOOL_LITERAL) {
            init_value = &init_expr->value.bln;
        } else {
            error_at(init_expr->token->filename, init_expr->token->line_no,
                "sorry, only literal initial values supported for now");
            return;
        }
    }

    ir_entry *declaration = new_ir_data_declaration(length, init_value, decl->var_name, storage);
    cg->ir->ops->add(cg->ir, declaration);
}

static void declare_expr_strings(code_gen *cg, ast_expression *expr) {
    if (expr == NULL) return;

    if (expr->op == OP_STR_LITERAL) {
        int num = cg->ops->next_label_num(cg);
        char sym_name[16];
        snprintf(sym_name, sizeof(sym_name) - 1, "__str_%d", num);
        sym_name[16] = 0;
        
        cg->ir->ops->add(cg->ir, new_ir_data_declaration(
            strlen(expr->value.str) + 1, expr->value.str, 
            sym_name, IR_GLOBAL_RO));

        // convert expression into symbol. will it work?
        expr->op = OP_SYMBOL_NAME;
        expr->value.str = strdup(sym_name);
    } else {
        // recurse
        declare_expr_strings(cg, expr->arg1);
        declare_expr_strings(cg, expr->arg2);
    }

}

static void declare_stmt_strings(code_gen *cg, ast_statement *stmt) {
    if (stmt == NULL) return;

    if (stmt->stmt_type == ST_BLOCK) {
        for (ast_statement *s = stmt->body; s != NULL; s = s->next)
            declare_stmt_strings(cg, s);
    } else if (stmt->stmt_type == ST_VAR_DECL && stmt->expr != NULL && stmt->expr->op == OP_STR_LITERAL) {
        // a variable initialized to a string
        cg->ir->ops->add(cg->ir, new_ir_data_declaration(
            strlen(stmt->expr->value.str) + 1, stmt->expr->value.str, 
            stmt->decl->var_name, IR_GLOBAL_RO));

        // maybe no further need for initalization?
        stmt->expr = NULL;
    } else {
        declare_expr_strings(cg, stmt->expr);
        declare_stmt_strings(cg, stmt->body);
        declare_stmt_strings(cg, stmt->else_body);
    }
}

void code_gen_generate_for_module(code_gen *cg, ast_module *module) {

    for_list(module->statements, ast_statement, stmt) {
        if (stmt->stmt_type != ST_VAR_DECL) {
            error_at(stmt->token->filename, stmt->token->line_no, "only var declarations are supported in code generation");
            return;
        }

        // generates strings as needed
        gen_global_var(cg, stmt->decl, stmt->expr);
        if (errors_count) return;
    }

    // traverse all to delcare any possible strings
    for_list(module->functions, ast_function, func) {
        for (ast_statement *s = func->stmts_list; s != NULL; s = s->next) {
            declare_stmt_strings(cg, s);
            if (errors_count) return;
        }
    }
    
    // generate code for all functions
    for_list(module->functions, ast_function, func) {
        if (func->stmts_list != NULL) { // maybe it's just a func declaration
            cg->ops->generate_for_function(cg, func);
            if (errors_count) return;
        }
    }
}

