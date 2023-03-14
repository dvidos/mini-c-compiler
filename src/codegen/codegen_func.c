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


static void traverse_and_generate_vars(code_gen *cg, statement *stmt) {
    if (stmt == NULL)
        return;
    
    switch (stmt->stmt_type) {
        case ST_VAR_DECL:
            // we have a bug, if same var declared in inner scopes, we don't currently support it.
            // we should have a stack of blocks and then have a way to resolve, same as with parser.
            cg->ir->ops->add(cg->ir, new_ir_data_declaration(
                stmt->decl->data_type->ops->size_of(stmt->decl->data_type),
                NULL, stmt->decl->var_name, IR_LOCAL, 0));
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

    cg->ir->ops->add(cg->ir, new_ir_function_definition(func->func_name));
    cg->ops->set_curr_func_name(cg, func->func_name);

    // register arguments as local variables
    int arg_no = 0;
    var_declaration *arg = func->args_list;
    while (arg != NULL) {
        cg->ir->ops->add(cg->ir, new_ir_data_declaration(
            arg->data_type->ops->size_of(arg->data_type),
            NULL, arg->var_name, IR_FUNC_ARG, arg_no++));
        arg = arg->next;
    }

    // traverse function tree to find local variables.
    statement *stmt = func->stmts_list;
    while (stmt != NULL) {
        traverse_and_generate_vars(cg, stmt);
        stmt = stmt->next;
    }

    // create a return-value symbol if needed
    if (func->return_type != NULL && func->return_type->family != TF_VOID) {
        cg->ir->ops->add(cg->ir, new_ir_data_declaration(
            func->return_type->ops->size_of(func->return_type),
            NULL, "ret_val", IR_RET_VAL, 0));
    }

    // generate code for function statements tree
    stmt = func->stmts_list;
     while (stmt != NULL) {
        cg->ops->generate_for_statement(cg, stmt);
        stmt = stmt->next;
    }

    cg->ir->ops->add(cg->ir, new_ir_label("%s_end", func->func_name));
}
