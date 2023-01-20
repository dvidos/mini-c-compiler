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

void print_ast() {
    // print each var
    // print each func
    ast_var_decl_node *v = vars_list;
    while (v != NULL) {
        printf("  variable %s\n", v->var_name);
        v = v->next;
    }
    ast_func_decl_node *f = funcs_list;
    while (f != NULL) {
        printf("  function %s()\n", f->func_name);
        f = f->next;
    }
}
