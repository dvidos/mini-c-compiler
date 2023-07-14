#include <stdio.h>
#include <stdarg.h>
#include "analysis.h"
#include "../../err_handler.h"
#include "../ast/all.h"
#include "../ast/all.h"
#include "../scope.h"
#include "../ast/all.h"
#include "../ast/all.h"

/*
    analysis has the following components:
    - symbol declaration check (not declared, already declared)
    - symbol resolution (ensure declared)
    - data type check (e.g. expressions assigned to vars, used in "if"s, or return value types)
*/


void perform_declaration_analysis(ast_variable *decl, int arg_no) {

    if (scope_symbol_declared_at_curr_level(decl->var_name)) {
        error_at(
            decl->token->filename,
            decl->token->line_no,
            "symbol \"%s\" already defined in this scope",
            decl->var_name
        );
        return;
    }

    scoped_symbol *sym;
    if (arg_no >= 0)
        sym = new_scoped_symbol_func_arg(decl->mempool, decl->var_name, decl->data_type, arg_no, decl->token);
    else
        sym = new_scoped_symbol(decl->mempool, decl->var_name, decl->data_type, SYM_VAR, decl->token);
    scope_declare_symbol(sym);
}

void perform_function_analysis(ast_function *func) {

    // functions are declared at their parent scope
    if (scope_symbol_declared_at_curr_level(func->func_name)) {
        error_at(
            func->token->filename,
            func->token->line_no,
            "function \"%s\" already defined", 
            func->func_name);
    } else {
        scoped_symbol *sym = new_scoped_symbol_func(func->mempool, func->func_name, func, func->token);
        scope_declare_symbol(sym);
    }

    scope_entered(func); // scope of function

    ast_variable *arg = func->args_list;
    int arg_no = 0;
    while (arg != NULL) {
        perform_declaration_analysis(arg, arg_no);
        arg = arg->next;
        arg_no++;
    }

    // functions have a list of statements as their body.
    ast_statement *stmt = func->stmts_list;
    while (stmt != NULL) {
        perform_statement_analysis(stmt);
        stmt = stmt->next;
    }

    scope_exited(); // exiting function
}

void perform_module_analysis(ast_module *ast) {
    scope_entered(NULL);

    for_list(ast->statements, ast_statement, stmt)
        perform_statement_analysis(stmt);
    
    for_list(ast->functions, ast_function, func)
        perform_function_analysis(func);

    scope_exited();
}
