#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../../run_info.h"
#include "../../err_handler.h"
#include "../lexer/token.h"
#include "../expression.h"
#include "../statement.h"
#include "../src_symbol.h"
#include "../declaration.h"
#include "codegen.h"
#include "ir_listing.h"


static void traverse_and_generate_vars(code_gen *cg, statement *stmt) {
    if (stmt == NULL)
        return;
    
    switch (stmt->stmt_type) {
        case ST_VAR_DECL:
            // we have a bug, if same var declared in inner scopes, we don't currently support it.
            // we should have a stack of blocks and then have a way to resolve, same as with parser.
            cg->ir->ops->add(cg->ir, new_ir_data_declaration(
                stmt->decl->data_type->ops->size_of(stmt->decl->data_type),
                NULL, stmt->decl->var_name, IR_LOCAL));
            break;

        case ST_BLOCK:
            for (statement *s = stmt->body; s != NULL; s = s->next) 
                traverse_and_generate_vars(cg, s);
            break;

        case ST_IF:
            traverse_and_generate_vars(cg, stmt->body);
            traverse_and_generate_vars(cg, stmt->else_body);
            break;

        case ST_WHILE:
            traverse_and_generate_vars(cg, stmt->body);
            break;

        case ST_CONTINUE: // fallthrough
        case ST_BREAK:
        case ST_RETURN:
        case ST_EXPRESSION:
            // nothing here
            break;    
    }
}

void code_gen_generate_for_function(code_gen *cg, func_declaration *func) {

    cg->ops->set_curr_func_name(cg, func->func_name);

    // prepare IR function definition arguments
    int args_len = 0;
    struct ir_entry_func_arg_info *args_arr = NULL;
    for (var_declaration *arg = func->args_list; arg != NULL; arg = arg->next)
        args_len++;
    if (args_len > 0) {
        args_arr = malloc(args_len * sizeof(struct ir_entry_func_arg_info));
        int i = 0;
        for (var_declaration *arg = func->args_list; arg != NULL; arg = arg->next) {
            args_arr[i].name = arg->var_name;
            args_arr[i].size = arg->data_type->ops->size_of(arg->data_type);
            i++;
        }
    }

    // prepare IR function return size
    int ret_val_size = 0;
    if (func->return_type != NULL && func->return_type->family != TF_VOID)
        ret_val_size = func->return_type->ops->size_of(func->return_type);

    // declare function (and return value pseudo-var)
    cg->ir->ops->add(cg->ir, new_ir_function_definition(func->func_name, args_arr, args_len, ret_val_size));


    // traverse function tree to find local variables.
    statement *stmt = func->stmts_list;
    while (stmt != NULL) {
        traverse_and_generate_vars(cg, stmt);
        stmt = stmt->next;
    }

    // generate code for function statements tree
    stmt = func->stmts_list;
     while (stmt != NULL) {
        cg->ops->generate_for_statement(cg, stmt);
        stmt = stmt->next;
    }

    cg->ir->ops->add(cg->ir, new_ir_function_end());
}
