#include <stddef.h>
#include <stdio.h>
#include "ast.h"
#include "declaration.h"
#include "utils.h"

// currently we do not have a "tree" in the correct sense of the word.
// if we wanted to do so, we need to make function declarations part of a statement.

// the lists below at file level
static ast_module_node ast_root;

static void print_statement_list(statement *list, int depth);


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


static void indent(int depth) {
    while (depth--)
        printf("    ");
}

static void print_expression_using_func_format(expression *expr) {
    if (expr == NULL) {
        printf("NULL");
    } else if (expr->op == OP_SYMBOL_NAME) {
        printf("%s", expr->value.str);
    } else if (expr->op == OP_STR_LITERAL) {
        printf("\"");
        print_pretty(expr->value.str, stdout);
        printf("\"");
    } else if (expr->op == OP_NUM_LITERAL) {
        printf("%ld", expr->value.num);
    } else if (expr->op == OP_CHR_LITERAL) {
        printf("'%c'", expr->value.chr);
    } else if (expr->op == OP_BOOL_LITERAL) {
        printf("%s", expr->value.bln ? "true" : "false");
    } else {
        printf("%s(", oper_debug_name(expr->op));
        print_expression_using_func_format(expr->arg1);
        if (!is_unary_operator(expr->op)) {
            printf(", ");
            print_expression_using_func_format(expr->arg2);
        }
        printf(")");
    }
}

static void print_expression_using_tree_format(expression *expr, int depth) {
    indent(depth);
    
    if (expr == NULL) {
        printf("NULL\n");
    } else if (expr->op == OP_SYMBOL_NAME) {
        printf("%s\n", expr->value.str);
    } else if (expr->op == OP_STR_LITERAL) {
        printf("\"%s\"\n", expr->value.str);
    } else if (expr->op == OP_NUM_LITERAL) {
        printf("%ld\n", expr->value.num);
    } else if (expr->op == OP_CHR_LITERAL) {
        printf("'%c'\n", expr->value.chr);
    } else if (expr->op == OP_BOOL_LITERAL) {
        printf("%s", expr->value.bln ? "true" : "false");
    } else {
        printf("%s()\n", oper_debug_name(expr->op));
        print_expression_using_tree_format(expr->arg1, depth + 1);
        if (!is_unary_operator(expr->op)) {
            print_expression_using_tree_format(expr->arg2, depth + 1);
        }
    }
}

static void print_statement(statement *st, int depth) {
    switch (st->stmt_type) {
        case ST_BLOCK:
            indent(depth - 1);
            printf("{\n");
            print_statement_list(st->body, depth);
            indent(depth - 1);
            printf("}\n");
            break;

        case ST_VAR_DECL:
            indent(depth);
            printf("local variable: %s %s", 
                st->decl->var_name, 
                st->decl->data_type->ops->to_string(st->decl->data_type));
            if (st->expr != NULL) {
                printf(" = ");
                print_expression_using_func_format(st->expr);
            }
            printf("\n");
            break;

        case ST_IF:
            indent(depth);
            printf("if (");
            print_expression_using_func_format(st->expr);
            printf(")\n");
            print_statement(st->body, depth+1);
            if (st->else_body != NULL) {
                indent(depth);
                printf("else\n");
                print_statement(st->else_body, depth+1);
            }
            break;

        case ST_WHILE:
            indent(depth);
            printf("while (");
            print_expression_using_func_format(st->expr);
            printf(")\n");
            print_statement(st->body, depth+1);
            break;

        case ST_BREAK:
            indent(depth);
            printf("break\n");
            break;

        case ST_CONTINUE:
            indent(depth);
            printf("continue\n");
            break;
            
        case ST_RETURN:
            indent(depth);
            printf("return");
            if (st->expr != NULL) {
                printf(" ");
                print_expression_using_func_format(st->expr);
            }
            printf("\n");
            break;
            
        case ST_EXPRESSION:
            //print_expression_using_tree_format(st->expr, depth);
            indent(depth);
            printf("expr ");
            print_expression_using_func_format(st->expr);
            printf("\n");
            break;

        default:
            indent(depth);
            printf("??? (unknown statement type)\n");
    }
}

static void print_statement_list(statement *list, int depth) {
    statement *stmt = list;
    while (stmt != NULL) {
        print_statement(stmt, depth);
        stmt = stmt->next;
    }
}

void print_ast() {

    printf("Abstract Syntax Tree\n");

    statement *stmt = ast_root.statements_list;
    while (stmt != NULL) {
        print_statement(stmt, 1);
        stmt = stmt->next;
    }

    func_declaration *func = ast_root.funcs_list;
    while (func != NULL) {
        indent(1);
        printf("function: ");
        printf("%s", func->return_type->ops->to_string(func->return_type));
        printf(" %s(", func->func_name);
        var_declaration *arg = func->args_list;
        while (arg != NULL) {
            printf("%s", arg->data_type->ops->to_string(arg->data_type));
            printf(" %s", arg->var_name);
            if (arg->next != NULL)
                printf(", ");
            arg = arg->next;
        }
        printf(")\n");
        
        // func body here...
        print_statement_list(func->stmts_list, 2);

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