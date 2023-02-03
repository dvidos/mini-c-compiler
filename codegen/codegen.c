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
#include "codegen.h"
#include "interm_repr.h"


/*
    For assembly opcodes one can see Intel's books
    https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
    
    Or this useful website:
    http://ref.x86asm.net/coder32.html


*/


struct function_code_generation_info current_function_generated;


void generate_function_code(func_declaration *func) {
    current_function_generated.decl = func;

    // stack frame, decoding of arguments, local data, etc.
    // where do we go from here?

    ir_set_next_label("%s", func->func_name);

    // allocate stack space for func arguments and locals
    // find all var declarations in the subtree

    statement *stmt = func->stmts_list;
    while (stmt != NULL) {
        generate_statement_code(stmt);
        stmt = stmt->next;
    }

    ir_set_next_label("%s_exit", func->func_name);
    ir_add_str("RET");
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
}

