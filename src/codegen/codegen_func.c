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



struct local_var_info {
    var_declaration *decl;
    bool is_arg;
    int arg_no;
    int bp_offset;
    int size_bytes;
};

struct curr_func_info {
    func_declaration *decl;

    struct local_var_info *vars;
    int vars_count;
    int vars_capacity;
};

static struct curr_func_info *curr_func = NULL;



static void gather_local_var_declarations(statement *stmt) {
    if (stmt == NULL)
        return;
    
    switch (stmt->stmt_type) {
        case ST_VAR_DECL:
            // bingo!
            cg.register_local_var(stmt->decl, false, 0);
            break;

        case ST_BLOCK:
            statement *inner = stmt->body;
            while (inner != NULL) {
                gather_local_var_declarations(inner);
                inner = inner->next;
            }
            break;

        case ST_IF:
            gather_local_var_declarations(stmt->body);
            gather_local_var_declarations(stmt->else_body);
            break;

        case ST_WHILE:
            gather_local_var_declarations(stmt->body);
            break;

        case ST_CONTINUE: // fallthrough
        case ST_BREAK:
        case ST_RETURN:
        case ST_EXPRESSION:
            break;    
    }
}

static void generate_local_var_code() {
    if (curr_func->vars_count == 0)
        return;

    for (int i = 0; i < curr_func->vars_count; i++) {
        struct local_var_info *var = &curr_func->vars[i];
        if (var->is_arg) {
            ir.add_comment("\t; arg #%d, %s %s, located at %cBP+%d", 
                var->arg_no,
                var->decl->data_type->ops->to_string(var->decl->data_type), 
                var->decl->var_name, 
                options.register_prefix,
                var->bp_offset);
        } else {
            ir.add_str("SUB %cSP, %d   ; %s %s, at %cBP%d", 
                options.register_prefix, 
                var->size_bytes,
                var->decl->data_type->ops->to_string(var->decl->data_type), 
                var->decl->var_name,
                options.register_prefix,
                var->bp_offset
            );
        }
    }
}

void generate_function_code(func_declaration *func) {

    cg.assign_curr_func(func);

    // register arguments as local variables
    int arg_no = 0;
    var_declaration *arg = func->args_list;
    while (arg != NULL) {
        cg.register_local_var(arg, true, arg_no++);
        arg = arg->next;
    }

    // traverse function tree to find local variables.
    statement *stmt = func->stmts_list;
    while (stmt != NULL) {
        gather_local_var_declarations(stmt);
        stmt = stmt->next;
    }

    // establish stack frame
    ir.set_next_label("%s", func->func_name);
    ir.add_str("PUSH %cBP", options.register_prefix);
    ir.add_str("MOV %cBP, %cSP", options.register_prefix, options.register_prefix);

    // generate code for stack allocation
    generate_local_var_code();

    // generate code for function statements tree
    stmt = func->stmts_list;
     while (stmt != NULL) {
        generate_statement_code(stmt);
        stmt = stmt->next;
    }

    // preparng to exit
    ir.set_next_label("%s_exit", func->func_name);
    ir.add_str("MOV %cSP, %cBP", options.register_prefix, options.register_prefix);
    ir.add_str("POP %cBP", options.register_prefix);
    ir.add_str("RET");
}
