#include <stdio.h>
#include <stdarg.h>
#include "../err_handler.h"
#include "../ast_node.h"
#include "../statement.h"
#include "../scope.h"
#include "../symbol.h"
#include "analysis.h"


void perform_statement_analysis(statement *stmt) {
    if (stmt == NULL)
        return;

    if (stmt->stmt_type == ST_BLOCK) {
        scope_entered(NULL);
        statement *s = stmt->body;
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
        func_declaration *curr_func = get_scope_owning_function();
        if (curr_func == NULL) {
            error(stmt->token->filename, stmt->token->line_no, "return outside of a function is not supported");
        } else if (curr_func->return_type->family == TF_VOID && stmt->expr != NULL) {
            error(stmt->token->filename, stmt->token->line_no, "cannot return a value in this function");
        } else if (curr_func->return_type->family != TF_VOID && stmt->expr == NULL) {
            error(stmt->token->filename, stmt->token->line_no, "return needs to provide a value in this function");
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

