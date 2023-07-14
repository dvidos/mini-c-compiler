#include <stdio.h>
#include <stdarg.h>
#include "../../err_handler.h"
#include "../ast/all.h"
#include "../ast/all.h"
#include "../scope.h"
#include "../ast/all.h"
#include "analysis.h"


void perform_statement_analysis(ast_statement *stmt) {
    if (stmt == NULL)
        return;

    if (stmt->stmt_type == ST_BLOCK) {
        scope_entered(NULL);
        ast_statement *s = stmt->body;
        while (s != NULL) {
            perform_statement_analysis(s);
            s = s->next;
        }
        scope_exited();

    } else if (stmt->stmt_type == ST_IF) {
        perform_expression_analysis(stmt->expr);
        verify_expr_result_boolean(stmt->expr);
        perform_statement_analysis(stmt->body);
        perform_statement_analysis(stmt->else_body);

    } else if (stmt->stmt_type == ST_WHILE) {
        perform_expression_analysis(stmt->expr);
        verify_expr_result_boolean(stmt->expr);
        perform_statement_analysis(stmt->body);

    } else if (stmt->stmt_type == ST_RETURN) {
        perform_expression_analysis(stmt->expr);
        ast_function *curr_func = get_scope_owning_function();
        if (curr_func == NULL) {
            error_at(stmt->token->filename, stmt->token->line_no, "return outside of a function is not supported");
        } else if (curr_func->return_type->family == TF_VOID && stmt->expr != NULL) {
            error_at(stmt->token->filename, stmt->token->line_no, "cannot return a value in this function");
        } else if (curr_func->return_type->family != TF_VOID && stmt->expr == NULL) {
            error_at(stmt->token->filename, stmt->token->line_no, "return needs to provide a value in this function");
        } else if (curr_func->return_type->family == TF_VOID && stmt->expr == NULL) {
            ; // we are good, returning no value on a void function
        } else {
            verify_expression_result_type(stmt->expr, curr_func->return_type);
        }

    } else if (stmt->stmt_type == ST_VAR_DECL) {
        // possible initialization expression
        perform_declaration_analysis(stmt->decl, -1);
        perform_expression_analysis(stmt->expr);
        if (stmt->expr != NULL)
            verify_expression_result_type(stmt->expr, stmt->decl->data_type);

    } else if (stmt->stmt_type == ST_EXPRESSION) {
        perform_expression_analysis(stmt->expr);

    } else if (stmt->stmt_type == ST_BREAK) {
        // nothing here
    } else if (stmt->stmt_type == ST_CONTINUE) {
        // nothing here
    }
}

