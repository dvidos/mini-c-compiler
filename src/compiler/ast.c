#include <stddef.h>
#include <stdio.h>
#include "ast.h"
#include "declaration.h"
#include "../utils.h"

// currently we do not have a "tree" in the correct sense of the word.
// if we wanted to do so, we need to make function declarations part of a statement.

// the lists below at file level
static ast_module_node ast_root;

static void print_statement_list(FILE *stream, statement *list, int depth);


void init_ast() {
    ast_root.statements_list = NULL;
    ast_root.funcs_list = NULL;
}

ast_module_node *get_ast_root_node() {
    return &ast_root;
}

void ast_add_statement(statement *stmt) {
    if (ast_root.statements_list == NULL) {
        ast_root.statements_list = stmt;
    } else {
        statement *p = ast_root.statements_list;
        while (p->next != NULL) p = p->next;
        p->next = stmt;
    }
    stmt->next = NULL;
}

void ast_add_function(func_declaration *func) {
    if (ast_root.funcs_list == NULL) {
        ast_root.funcs_list = func;
    } else {
        func_declaration *p = ast_root.funcs_list;
        while (p->next != NULL) p = p->next;
        p->next = func;
    }
    func->next = NULL;
}


static void indent(FILE *stream, int depth) {
    while (depth--)
        fprintf(stream, "    ");
}

static void print_expression_using_func_format(FILE *stream, expression *expr) {
    if (expr == NULL) {
        fprintf(stream, "NULL");
    } else if (expr->op == OP_SYMBOL_NAME) {
        fprintf(stream, "%s", expr->value.str);
    } else if (expr->op == OP_STR_LITERAL) {
        fprintf(stream, "\"");
        print_pretty(expr->value.str, stream);
        fprintf(stream, "\"");
    } else if (expr->op == OP_NUM_LITERAL) {
        fprintf(stream, "%ld", expr->value.num);
    } else if (expr->op == OP_CHR_LITERAL) {
        fprintf(stream, "'%c'", expr->value.chr);
    } else if (expr->op == OP_BOOL_LITERAL) {
        fprintf(stream, "%s", expr->value.bln ? "true" : "false");
    } else {
        fprintf(stream, "%s(", oper_debug_name(expr->op));
        print_expression_using_func_format(stream, expr->arg1);
        if (!is_unary_operator(expr->op)) {
            fprintf(stream, ", ");
            print_expression_using_func_format(stream, expr->arg2);
        }
        fprintf(stream, ")");
    }
}

static void print_expression_using_tree_format(FILE *stream, expression *expr, int depth) {
    indent(stream, depth);
    
    if (expr == NULL) { fprintf(stream, "NULL\n");
    } else if (expr->op == OP_SYMBOL_NAME)  { fprintf(stream, "%s\n", expr->value.str);
    } else if (expr->op == OP_STR_LITERAL)  { fprintf(stream, "\"%s\"\n", expr->value.str);
    } else if (expr->op == OP_NUM_LITERAL)  { fprintf(stream, "%ld\n", expr->value.num);
    } else if (expr->op == OP_CHR_LITERAL)  { fprintf(stream, "'%c'\n", expr->value.chr);
    } else if (expr->op == OP_BOOL_LITERAL) { fprintf(stream, "%s", expr->value.bln ? "true" : "false");
    } else {
        fprintf(stream, "%s()\n", oper_debug_name(expr->op));
        print_expression_using_tree_format(stream, expr->arg1, depth + 1);
        if (!is_unary_operator(expr->op)) {
            print_expression_using_tree_format(stream, expr->arg2, depth + 1);
        }
    }
}

static void print_statement(FILE *stream, statement *st, int depth) {
    switch (st->stmt_type) {
        case ST_BLOCK:
            print_statement_list(stream, st->body, depth);
            break;

        case ST_VAR_DECL:
            indent(stream, depth);
            fprintf(stream, "variable: %s %s", 
                st->decl->var_name, 
                st->decl->data_type->ops->to_string(st->decl->data_type));
            if (st->expr != NULL) {
                fprintf(stream, " = ");
                print_expression_using_func_format(stream, st->expr);
            }
            fprintf(stream, "\n");
            break;

        case ST_IF:
            indent(stream, depth);
            fprintf(stream, "if (");
            print_expression_using_func_format(stream, st->expr);
            fprintf(stream, ")\n");
            print_statement(stream, st->body, depth+1);
            if (st->else_body != NULL) {
                indent(stream, depth);
                fprintf(stream, "else\n");
                print_statement(stream, st->else_body, depth+1);
            }
            break;

        case ST_WHILE:
            indent(stream, depth);
            fprintf(stream, "while (");
            print_expression_using_func_format(stream, st->expr);
            fprintf(stream, ")\n");
            print_statement(stream, st->body, depth+1);
            break;

        case ST_BREAK:
            indent(stream, depth);
            fprintf(stream, "break\n");
            break;

        case ST_CONTINUE:
            indent(stream, depth);
            fprintf(stream, "continue\n");
            break;
            
        case ST_RETURN:
            indent(stream, depth);
            fprintf(stream, "return");
            if (st->expr != NULL) {
                fprintf(stream, " ");
                print_expression_using_func_format(stream, st->expr);
            }
            fprintf(stream, "\n");
            break;
            
        case ST_EXPRESSION:
            //print_expression_using_tree_format(st->expr, depth);
            indent(stream, depth);
            // fprintf(stream, "expr ");
            print_expression_using_func_format(stream, st->expr);
            fprintf(stream, "\n");
            break;

        default:
            indent(stream, depth);
            fprintf(stream, "??? (unknown statement type)\n");
    }
}

static void print_statement_list(FILE *stream, statement *list, int depth) {
    statement *stmt = list;
    while (stmt != NULL) {
        print_statement(stream, stmt, depth);
        stmt = stmt->next;
    }
}

void print_ast(FILE *stream) {

    statement *stmt = ast_root.statements_list;
    while (stmt != NULL) {
        print_statement(stream, stmt, 0);
        stmt = stmt->next;
    }

    func_declaration *func = ast_root.funcs_list;
    while (func != NULL) {
        fprintf(stream, "\n");
        indent(stream, 0);
        fprintf(stream, "function: %s %s(",
            func->return_type->ops->to_string(func->return_type),
            func->func_name);
        var_declaration *arg = func->args_list;
        while (arg != NULL) {
            fprintf(stream, "%s %s",
                 arg->data_type->ops->to_string(arg->data_type),
                 arg->var_name);
            if (arg->next != NULL)
                fprintf(stream, ", ");
            arg = arg->next;
        }
        fprintf(stream, ")\n");
        
        // func body here...
        print_statement_list(stream, func->stmts_list, 1);

        func = func->next;
    }
}


static void ast_count_expression_nodes(expression *expr, int *expressions) {
    if (expr == NULL)
        return;
    (*expressions)++;
    ast_count_expression_nodes(expr->arg1, expressions);
    ast_count_expression_nodes(expr->arg2, expressions);
}


static void ast_count_statements(statement *stmt, int *statements, int *expressions) {
    if (stmt == NULL)
        return;
    (*statements)++;

    if (stmt->decl != NULL)
        (*statements)++;

    ast_count_expression_nodes(stmt->expr, expressions);

    statement *s = stmt->body;
    while (s != NULL) {
        ast_count_statements(s, statements, expressions);
        s = s->next;
    }
    s = stmt->else_body;
    while (s != NULL) {
        ast_count_statements(s, statements, expressions);
        s = s->next;
    }
}

void ast_count_nodes(int *functions, int *statements, int *expressions) {
    (*functions) = 0;
    (*statements) = 0;
    (*expressions) = 0;

    statement *stmt = ast_root.statements_list;
    while (stmt != NULL) {
        (*statements)++;
        stmt = stmt->next;
    }

    func_declaration *func = ast_root.funcs_list;
    while (func != NULL) {
        (*functions)++;
        // TODO: fix this so that function bodies count correct
        ast_count_statements(func->stmts_list, statements, expressions);
        func = func->next;
    }
}