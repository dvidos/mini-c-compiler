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
#include "interm_repr.h"
#include "ir_listing.h"


static void generate_local_var_declarations(ir_listing *l, statement *stmt) {
    if (stmt == NULL)
        return;
    
    switch (stmt->stmt_type) {
        case ST_VAR_DECL:
            // bingo!
            cg.register_local_var(stmt->decl, false, 0);
            l->ops->add(l, new_ir_data_declaration(
                stmt->decl->data_type->ops->size_of(stmt->decl->data_type),
                NULL, // maybe compile time initial data? stmt->expr
                stmt->decl->var_name,
                IR_LOCAL));
            break;

        case ST_BLOCK:
            statement *inner = stmt->body;
            while (inner != NULL) {
                generate_local_var_declarations(l, inner);
                inner = inner->next;
            }
            break;

        case ST_IF:
            generate_local_var_declarations(l, stmt->body);
            generate_local_var_declarations(l, stmt->else_body);
            break;

        case ST_WHILE:
            generate_local_var_declarations(l, stmt->body);
            break;

        case ST_CONTINUE: // fallthrough
        case ST_BREAK:
        case ST_RETURN:
        case ST_EXPRESSION:
            // nothing here
            break;    
    }
}

void generate_function_ir_code(ir_listing *listing, func_declaration *func) {

    cg.assign_curr_func(func);

    listing->ops->add(listing, new_ir_label(func->func_name));

    // register arguments as local variables
    int arg_no = 0;
    var_declaration *arg = func->args_list;
    while (arg != NULL) {
        cg.register_local_var(arg, true, arg_no++);
        listing->ops->add(listing, new_ir_data_declaration(
            arg->data_type->ops->size_of(arg->data_type),
            NULL,
            arg->var_name,
            IR_FUNC_ARG));
        arg = arg->next;
    }

    // traverse function tree to find local variables.
    statement *stmt = func->stmts_list;
    while (stmt != NULL) {
        generate_local_var_declarations(listing, stmt);
        stmt = stmt->next;
    }

    if (func->return_type != NULL && func->return_type->family != TF_VOID) {
        listing->ops->add(listing, new_ir_data_declaration(
            func->return_type->ops->size_of(func->return_type),
            NULL,
            "ret_val",
            IR_RET_VAL));
    }

    // generate code for function statements tree
    stmt = func->stmts_list;
     while (stmt != NULL) {
        generate_statement_ir_code(listing, stmt);
        stmt = stmt->next;
    }

    listing->ops->add(listing, new_ir_comment("end of function"));
}
