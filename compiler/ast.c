#include <stddef.h>
#include <stdio.h>
#include "ast_node.h"

// the lists below at file level
ast_statement_node *statements_list;
ast_func_decl_node *funcs_list;

void init_ast() {
    statements_list = NULL;
    funcs_list = NULL;
}

void ast_add_statement(ast_statement_node *stmt) {
    if (statements_list == NULL) {
        statements_list = stmt;
    } else {
        ast_statement_node *p = statements_list;
        while (p->next != NULL) p = p->next;
        p->next = stmt;
    }
    stmt->next = NULL;
}

void ast_add_function(ast_func_decl_node *func) {
    if (funcs_list == NULL) {
        funcs_list = func;
    } else {
        ast_func_decl_node *p = funcs_list;
        while (p->next != NULL) p = p->next;
        p->next = func;
    }
    func->next = NULL;
}


static void indent(int depth) {
    while (depth--)
        printf(".  ");
}

static void print_data_type(ast_data_type_node *n) {
    printf("%s", data_type_family_name(n->family));
    if (n->nested != NULL) {
        printf("(");
        print_data_type(n->nested);
        printf(")");
    }
}

static void print_expression_using_func_format(ast_expression_node *expr) {
    if (expr == NULL) {
        printf("NULL");
    } else if (expr->op == OP_SYMBOL_NAME) {
        printf("%s", expr->value.str);
    } else if (expr->op == OP_STR_LITERAL) {
        printf("\"%s\"", expr->value.str);
    } else if (expr->op == OP_NUM_LITERAL) {
        printf("%ld", expr->value.num);
    } else if (expr->op == OP_CHR_LITERAL) {
        printf("'%c'", expr->value.chr);
    } else {
        printf("%s(", oper_debug_name(expr->op));
        print_expression_using_func_format(expr->arg1);
        if (!is_unary_operator(expr->op)) {
            printf(",");
            print_expression_using_func_format(expr->arg2);
        }
        printf(")");
    }
}

static void print_expression_using_tree_format(ast_expression_node *expr, int depth) {
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
    } else {
        printf("%s()\n", oper_debug_name(expr->op));
        print_expression_using_tree_format(expr->arg1, depth + 1);
        if (!is_unary_operator(expr->op)) {
            print_expression_using_tree_format(expr->arg2, depth + 1);
        }
    }
}

static void print_statement(ast_statement_node *st, int depth) {
    switch (st->stmt_type) {
        case ST_BLOCK:
            indent(depth - 1);
            printf("{\n");
            ast_statement_node *s = st->body;
            while (s != NULL) {
                print_statement(s, depth);
                s = s->next;
            }
            indent(depth - 1);
            printf("}\n");
            break;

        case ST_DECLARATION:
            indent(depth);
            print_data_type(st->decl->data_type);
            printf(" %s", st->decl->var_name);
            if (st->eval != NULL) {
                printf("=");
                print_expression_using_func_format(st->eval);
            }
            printf("\n");
            break;

        case ST_IF:
            indent(depth);
            printf("if(");
            print_expression_using_func_format(st->eval);
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
            printf("while(");
            print_expression_using_func_format(st->eval);
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
            if (st->eval != NULL) {
                printf(" (");
                print_expression_using_func_format(st->eval);
                printf(")");
            }
            printf("\n");
            break;
            
        case ST_EXPRESSION:
            print_expression_using_tree_format(st->eval, depth);
            // indent(depth);
            // print_expression_using_func_format(st->eval);
            break;

        default:
            indent(depth);
            printf("??? (unknown statement type)\n");
    }
}

void print_ast() {

    ast_statement_node *stmt = statements_list;
    while (stmt != NULL) {
        print_statement(stmt, 0);
        stmt = stmt->next;
    }

    ast_func_decl_node *func = funcs_list;
    while (func != NULL) {
        printf("function: ");
        print_data_type(func->return_type);
        printf(" %s(", func->func_name);
        ast_var_decl_node *arg = func->args_list;
        while (arg != NULL) {
            print_data_type(arg->data_type);
            printf(" %s", arg->var_name);
            if (arg->next != NULL)
                printf(", ");
            arg = arg->next;
        }
        printf(")\n");

        // func body here...
        if (func->body != NULL) {
            print_statement(func->body, 1);
        }

        func = func->next;
    }
}
