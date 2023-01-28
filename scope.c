#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "scope.h"
#include "symbol.h"
#include "ast_node.h"

// a stack of scopes, the outermost pushed first
scope *scopes_stack_top = NULL;

// creates a new scope on the stack
void scope_entered(ast_func_decl_node *func) {
    scope *s = malloc(sizeof(scope));
    s->symbols_list_head = NULL;
    s->symbols_list_tail = NULL;
    s->scoped_func = func;
    
    // add it to stack
    s->higher = scopes_stack_top;
    scopes_stack_top = s;
}

// pop a scope from the stack
void scope_exited() {
    if (scopes_stack_top == NULL) {
        printf("Too many scopes exited!\n");
        return;
    }

    scope *top = scopes_stack_top;
    scopes_stack_top = top->higher;

    if (top->symbols_list_head != NULL) {
        if (top->scoped_func != NULL) {
            printf("Symbols list of function %s()\n", top->scoped_func->func_name);
        } else if (top->higher == NULL) {
            printf("Symbols list of file scope\n");
        } else {
            printf("Symbols list for block\n");
        }
        symbol *sym = top->symbols_list_head;
        while (sym != NULL) {
            printf("- %-4s %-10s %s\n", symbol_type_name(sym->sym_type), data_type_to_string(sym->data_type), sym->name);
            sym = sym->next;
        }
    }
    // maybe we should also free all symbols???
    free(top);
}

// find a symbol in all scopes
symbol *scope_lookup(char *symbol_name) {
    scope *sc = scopes_stack_top;
    while (sc != NULL) {
        // look up in this scope
        symbol *sym = sc->symbols_list_head;
        while (sym != NULL) {
            if (strcmp(sym->name, symbol_name) == 0)
                return sym;
            sym = sym->next;
        }
        // not found, try wider scope
        sc = sc->higher;
    }
    return NULL;
}

ast_func_decl_node *get_function_in_scope() {
    scope *sc = scopes_stack_top;
    while (sc != NULL) {
        if (sc->scoped_func != NULL)
            return sc->scoped_func;
        
        // not found, try wider scope
        sc = sc->higher;
    }
    return NULL;
}

// see if symbol already declared
bool scope_symbol_declared_at_curr_level(char *symbol_name) {
    if (scopes_stack_top == NULL)
        return false;

    // look up in this scope
    symbol *sym = scopes_stack_top->symbols_list_head;
    while (sym != NULL) {
        if (strcmp(sym->name, symbol_name) == 0)
            return true;
        sym = sym->next;
    }

    return false;
}

// declare a symboll
void scope_declare_symbol(symbol *symbol) {
    if (scopes_stack_top == NULL)
        return;
    if (scopes_stack_top->symbols_list_tail == NULL) {
        scopes_stack_top->symbols_list_head = symbol;
        scopes_stack_top->symbols_list_tail = symbol;
    } else {
        scopes_stack_top->symbols_list_tail->next = symbol;
        scopes_stack_top->symbols_list_tail = symbol;
    }
    symbol->next = NULL;
}

