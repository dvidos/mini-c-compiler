#include <stdio.h>
#include <stdarg.h>
#include "analysis.h"
#include "../../err_handler.h"
#include "../ast_declaration.h"
#include "../ast_statement.h"
#include "../scope.h"
#include "../src_symbol.h"
#include "../ast_module.h"

/*
    analysis has the following components:
    - symbol declaration check (not declared, already declared)
    - symbol resolution (ensure declared)
    - data type check (e.g. expressions assigned to vars, used in "if"s, or return value types)
*/


void perform_declaration_analysis(ast_var_declaration *decl, int arg_no) {

    if (scope_symbol_declared_at_curr_level(decl->var_name)) {
        error_at(
            decl->token->filename,
            decl->token->line_no,
            "symbol \"%s\" already defined in this scope",
            decl->var_name
        );
        return;
    }

    src_symbol *sym;
    if (arg_no >= 0)
        sym = create_func_arg_symbol(decl->var_name, decl->data_type, arg_no, decl->token->filename, decl->token->line_no);
    else
        sym = create_symbol(decl->var_name, decl->data_type, SYM_VAR, decl->token->filename, decl->token->line_no);
    scope_declare_symbol(sym);
}

void perform_function_analysis(ast_func_declaration *func) {

    // functions are declared at their parent scope
    if (scope_symbol_declared_at_curr_level(func->func_name)) {
        error_at(
            func->token->filename,
            func->token->line_no,
            "function \"%s\" already defined", 
            func->func_name);
    } else {
        src_symbol *sym = create_func_symbol(func->func_name, func, func->token->filename, func->token->line_no);
        scope_declare_symbol(sym);
    }

    scope_entered(func); // scope of function

    ast_var_declaration *arg = func->args_list;
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

void perform_module_analysis(ast_module *ast_root) {
    scope_entered(NULL);

    ast_statement *stmt = ast_root->statements_list_head;
    while (stmt != NULL) {
        perform_statement_analysis(stmt);
        stmt = stmt->next;
    }

    ast_func_declaration *func = ast_root->funcs_list_head;
    while (func != NULL) {
        perform_function_analysis(func);
        func = func->next;
    }

    scope_exited();
}
