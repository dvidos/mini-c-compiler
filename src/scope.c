#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "options.h"
#include "scope.h"
#include "src_symbol.h"
#include "declaration.h"

// a stack of scopes, the outermost pushed first
scope *scopes_stack_top = NULL;

// creates a new scope on the stack
void scope_entered(func_declaration *func) {
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

    if (options.verbose)
        print_symbol_table(top);

    // maybe we should also free all symbols???
    free(top);
}

// find a symbol in all scopes
src_symbol *scope_lookup(char *symbol_name) {
    scope *sc = scopes_stack_top;
    while (sc != NULL) {
        // look up in this scope
        src_symbol *sym = sc->symbols_list_head;
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

func_declaration *get_scope_owning_function() {
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
    src_symbol *sym = scopes_stack_top->symbols_list_head;
    while (sym != NULL) {
        if (strcmp(sym->name, symbol_name) == 0)
            return true;
        sym = sym->next;
    }

    return false;
}

// declare a symboll
void scope_declare_symbol(src_symbol *symbol) {
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

void print_symbol_table(scope *s) {
    if (s->symbols_list_head == NULL)
        return; // no symbols defined

    if (s->scoped_func != NULL)
        printf("Symbol table for function %s()\n", s->scoped_func->func_name);
    else if (s->higher == NULL)
        printf("Symbol table for file scope\n");
    else
        printf("Symbol table for block\n");

    src_symbol *sym = s->symbols_list_head;
    while (sym != NULL) {
        printf("    %-20s  %-5s %-5s\n", sym->name, symbol_type_name(sym->sym_type), sym->data_type->ops->to_string(sym->data_type));
        sym = sym->next;
    }
}

