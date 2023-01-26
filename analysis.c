#include <stdio.h>
#include "analysis.h"
#include "ast_node.h"
#include "scope.h"
#include "symbol.h"

static void perform_declaration_analysis(ast_var_decl_node *decl, int arg_no) {

    if (scope_symbol_declared_at_curr_level(decl->var_name)) {
        printf("Symbol \"%s\" already defined in this scope\n", decl->var_name);
        return;
    } else {
        symbol *sym;
        if (arg_no >= 0)
            sym = create_func_arg_symbol(decl->var_name, decl->data_type, arg_no);
        else
            sym = create_symbol(decl->var_name, decl->data_type, SYM_VAR);
        scope_declare_symbol(sym);
    }
}

static void perform_expression_analysis(expr_node *expr) {
    if (expr == NULL)
        return;
    
    // expression analyses are performed in a post-order manner,
    // to make sure the innermost expressions are verified first.
    perform_expression_analysis(expr->arg1);
    perform_expression_analysis(expr->arg2);

    // now we are talking..
    // see if identifiers are declared
    // see if the data types match what the operator expects or provides
}

void perform_statement_analysis(ast_statement_node *stmt) {
    if (stmt == NULL)
        return;

    if (stmt->stmt_type == ST_BLOCK) {
        scope_entered();
        ast_statement_node *s = stmt->body;
        while (s != NULL) {
            perform_statement_analysis(s);
            s = s->next;
        }
        scope_exited();

    } else if (stmt->stmt_type == ST_IF) {
        perform_expression_analysis(stmt->eval);
        perform_statement_analysis(stmt->body);
        perform_statement_analysis(stmt->else_body);

    } else if (stmt->stmt_type == ST_WHILE) {
        perform_expression_analysis(stmt->eval);
        perform_statement_analysis(stmt->body);

    } else if (stmt->stmt_type == ST_RETURN) {
        // possible return value expression
        perform_expression_analysis(stmt->eval);

    } else if (stmt->stmt_type == ST_VAR_DECL) {
        // possible initialization expression
        perform_declaration_analysis(stmt->decl, -1);
        perform_expression_analysis(stmt->eval);

    } else if (stmt->stmt_type == ST_EXPRESSION) {
        perform_expression_analysis(stmt->eval);

    } else if (stmt->stmt_type == ST_BREAK) {
        // nothing here
    } else if (stmt->stmt_type == ST_CONTINUE) {
        // nothing here
    }
}

void perform_function_analysis(ast_func_decl_node *func) {
    scope_entered();

    if (scope_symbol_declared_at_curr_level(func->func_name)) {
        printf("Function \"%s\" already defined in current scope\n", func->func_name);
    } else {
        symbol *sym = create_symbol(func->func_name, func->return_type, SYM_FUNC);
        scope_declare_symbol(sym);
    }

    ast_var_decl_node *arg = func->args_list;
    int arg_no = 0;
    while (arg != NULL) {
        perform_declaration_analysis(arg, arg_no);
        arg = arg->next;
        arg_no++;
    }
    perform_statement_analysis(func->body);

    scope_exited();
}
