#include <stdio.h>
#include <stdarg.h>
#include "defs.h"
#include "analysis.h"
#include "ast_node.h"
#include "scope.h"
#include "symbol.h"

static bool analysis_issues_found = false;

static void analysis_error(char *file, int line, char *msg, ...) {

    printf("%s:%d: error: ", file, line);
    va_list args;
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
    printf("\n");

    analysis_issues_found = true;
}


static void perform_declaration_analysis(ast_var_decl_node *decl, int arg_no) {

    if (scope_symbol_declared_at_curr_level(decl->var_name)) {
        analysis_error(
            decl->token->filename,
            decl->token->line_no,
            "symbol \"%s\" already defined in this scope",
            decl->var_name
        );
        return;
    }

    symbol *sym;
    if (arg_no >= 0)
        sym = create_func_arg_symbol(decl->var_name, decl->data_type, arg_no, decl->token->filename, decl->token->line_no);
    else
        sym = create_symbol(decl->var_name, decl->data_type, SYM_VAR, decl->token->filename, decl->token->line_no);
    scope_declare_symbol(sym);
}

static void perform_expression_analysis(expr_node *expr) {
    if (expr == NULL)
        return;
    
    // expression analyses are performed in a post-order manner,
    // to make sure the innermost expressions are verified first.
    perform_expression_analysis(expr->arg1);
    perform_expression_analysis(expr->arg2);

    // see if identifiers are declared
    // see if the data types match what the operator expects or provides
    if (expr->op == OP_SYMBOL_NAME) {
        symbol *s = scope_lookup(expr->value.str);
        if (s == NULL) {
            analysis_error(
                expr->token->filename,
                expr->token->line_no,
                "symbol \"%s\" not declared",
                expr->value.str
            );
        }
    }
    
    /*  some ideas to explore
        - expression to be assigned must have the same type as the target lvalue
        - arguments passed in functions must match the argument's type
        - return value should match the return type of the function
        - all binary operators must have the same type left and right
        - (in)equality operators can not be applied to arrays, void.
        - (in)equality operators always return boolean
        - comparison > >= < <= operators can be applied to ints and return bool
          (could / should we apply to char? float? pointers?)
        - arithmetic operators + - * / etc apply to int/float and return int/float
    */
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
        printf("%s:%d: function \"%s\" already defined in current scope\n", 
            func->token->filename,
            func->token->line_no,
            func->func_name);
    } else {
        symbol *sym = create_symbol(func->func_name, func->return_type, SYM_FUNC, func->token->filename, func->token->line_no);
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

int perform_module_analysis(ast_module_node *ast_root) {
    analysis_issues_found = false;
    scope_entered();

    ast_statement_node *s = ast_root->statements_list;
    while (s != NULL) {
        perform_statement_analysis(s);
        s = s->next;
    }
    ast_func_decl_node *f = ast_root->funcs_list;
    while (f != NULL) {
        perform_function_analysis(f);
        f = f->next;
    }

    scope_exited();
    return analysis_issues_found ? ERROR : SUCCESS;
}