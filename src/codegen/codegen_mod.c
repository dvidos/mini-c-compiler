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
#include "../src_symbol.h"
#include "../declaration.h"
#include "codegen.h"
#include "ir_listing.h"



static void gen_global_var(code_gen *cg, var_declaration *decl, expression *init_expr) {
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

static void declare_expr_strings(code_gen *cg, expression *expr) {
    if (expr == NULL) return;

    if (expr->op == OP_STR_LITERAL) {
        int num = cg->ops->next_label_num(cg);
        char sym_name[16];
        snprintf(sym_name, sizeof(sym_name) - 1, "__str_%d", num);
        sym_name[16] = 0;
        
        cg->ir->ops->add(cg->ir, new_ir_data_declaration(
            strlen(expr->value.str) + 1, expr->value.str, 
            sym_name, IR_GLOBAL_RO, 0));

        // convert expression into symbol. will it work?
        expr->op = OP_SYMBOL_NAME;
        expr->value.str = strdup(sym_name);

    } else {
        // recurse
        declare_expr_strings(cg, expr->arg1);
        declare_expr_strings(cg, expr->arg2);
    }

}

static void declare_stmt_strings(code_gen *cg, statement *stmt) {
    if (stmt == NULL) return;

    if (stmt->stmt_type == ST_BLOCK) {
        for (statement *s = stmt->body; s != NULL; s = s->next)
            declare_stmt_strings(cg, s);
    } else {
        declare_expr_strings(cg, stmt->expr);
        declare_stmt_strings(cg, stmt->body);
        declare_stmt_strings(cg, stmt->else_body);
    }
}

void code_gen_generate_for_module(code_gen *cg, ast_module_node *module) {

    statement *stmt = module->statements_list;
    while (stmt != NULL) {
        declare_stmt_strings(cg, stmt);

        if (stmt->stmt_type != ST_VAR_DECL) {
            error(stmt->token->filename, stmt->token->line_no, "only var declarations are supported in code generation");
            return;
        }

        gen_global_var(cg, stmt->decl, stmt->expr);
        stmt = stmt->next;
    }

    // traverse all to delcare any possible strings
    for (func_declaration *f = module->funcs_list; f != NULL; f = f->next) {
        for (statement *s = f->stmts_list; s != NULL; s = s->next)
            declare_stmt_strings(cg, s);
    }
    
    // generate code for all functions
    for (func_declaration *f = module->funcs_list; f != NULL; f = f->next) {
        if (f->stmts_list != NULL) // maybe just a declaration
            cg->ops->generate_for_function(cg, f);
    }
}

