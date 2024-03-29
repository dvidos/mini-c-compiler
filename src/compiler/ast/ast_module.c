#include <stddef.h>
#include <stdio.h>
#include "ast_module.h"
#include "../../utils.h"


static void print_statement_list(FILE *stream, ast_statement *list, int depth);


ast_module *new_ast_module(mempool *mp) {
    ast_module *m = mpalloc(mp, ast_module);
    m->statements = new_list(mp);
    m->functions = new_list(mp);
    m->mempool = mp;
    return m;
}

void ast_module_add_statement(ast_module *m, ast_statement *stmt) {
    list_add(m->statements, stmt);
}

void ast_module_add_function(ast_module *m, ast_function *func) {
    list_add(m->functions, func);
}

static void indent(FILE *stream, int depth) {
    while (depth--)
        fprintf(stream, "    ");
}

static void print_expression_using_func_format(FILE *stream, ast_expression *expr) {
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

static void print_expression_using_tree_format(FILE *stream, ast_expression *expr, int depth) {
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

static void print_statement(FILE *stream, ast_statement *st, int depth) {
    switch (st->stmt_type) {
        case ST_BLOCK:
            print_statement_list(stream, st->body, depth);
            break;

        case ST_VAR_DECL:
            indent(stream, depth);
            fprintf(stream, "variable: %s (%s)", 
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

static void print_statement_list(FILE *stream, ast_statement *list, int depth) {
    ast_statement *stmt = list;
    while (stmt != NULL) {
        print_statement(stream, stmt, depth);
        stmt = stmt->next;
    }
}

void ast_module_print(ast_module *m, FILE *stream) {
    for_list(m->statements, ast_statement, stmt)
        print_statement(stream, stmt, 0);
    
    for_list(m->functions, ast_function, func) {
        fprintf(stream, "\n");
        indent(stream, 0);
        fprintf(stream, "function: %s %s(",
            func->return_type->ops->to_string(func->return_type),
            func->func_name);
        ast_variable *arg = func->args_list;
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

static void ast_count_expression_nodes(ast_expression *expr, int *expressions) {
    if (expr == NULL)
        return;
    (*expressions)++;
    ast_count_expression_nodes(expr->arg1, expressions);
    ast_count_expression_nodes(expr->arg2, expressions);
}

static void ast_count_statements(ast_statement *stmt, int *statements, int *expressions) {
    if (stmt == NULL)
        return;
    (*statements)++;

    if (stmt->decl != NULL)
        (*statements)++;

    ast_count_expression_nodes(stmt->expr, expressions);

    ast_statement *s = stmt->body;
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

void ast_module_count_nodes(ast_module *m, int *functions, int *statements, int *expressions) {
    (*statements) = list_length(m->statements);
    (*functions) = list_length(m->functions);

    (*expressions) = 0;
    for_list(m->functions, ast_function, f)
        ast_count_statements(f->stmts_list, statements, expressions);
}