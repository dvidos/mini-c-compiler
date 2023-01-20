#include <stddef.h>
#include <stdio.h>
#include "ast_node.h"

// the lists below at file level
ast_var_decl_node *vars_list;
ast_func_decl_node *funcs_list;

void init_ast() {
    vars_list = NULL;
    funcs_list = NULL;
}

void ast_add_var(ast_var_decl_node *var) {
    if (vars_list == NULL) {
        vars_list = var;
    } else {
        ast_var_decl_node *p = vars_list;
        while (p->next != NULL) p = p->next;
        p->next = var;
    }
    var->next = NULL;
}

void ast_add_func(ast_func_decl_node *func) {
    if (funcs_list == NULL) {
        funcs_list = func;
    } else {
        ast_func_decl_node *p = funcs_list;
        while (p->next != NULL) p = p->next;
        p->next = func;
    }
    func->next = NULL;
}


static void print_ast_data_type(ast_data_type_node *n) {
    printf(data_type_family_name(n->family));
    if (n->nested != NULL) {
        printf("(");
        print_ast_data_type(n->nested);
        printf(")");
    }
}

void print_ast() {

    // print each var
    // print each func
    ast_var_decl_node *var = vars_list;
    while (var != NULL) {
        printf("  variable %s ", var->var_name);
        print_ast_data_type(var->data_type);
        printf("\n");
        var = var->next;
    }

    ast_func_decl_node *func = funcs_list;
    while (func != NULL) {
        printf("  function %s(), returns ", func->func_name);
        print_ast_data_type(func->return_type);
        printf("\n");
        if (func->args_list != NULL) {
            printf("    args list: ");
            ast_var_decl_node *arg = func->args_list;
            while (arg != NULL) {
                printf("\"%s\"", arg->var_name);
                printf(" ");
                print_ast_data_type(arg->data_type);
                if (arg->next != NULL)
                    printf(", ");
                arg = arg->next;
            }
            printf("\n");
        }

        // func body here...

        func = func->next;
    }
}
