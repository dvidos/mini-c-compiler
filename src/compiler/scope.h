#include <stddef.h>
#include "ast_symbol.h"
#include "ast_declaration.h"

typedef struct scope {
    ast_symbol *symbols_list_head;
    ast_symbol *symbols_list_tail;

    ast_func_declaration *scoped_func;
    struct scope *higher;
} scope;



void scope_entered(ast_func_declaration *func); // creates a new scope on the stack
void scope_exited(); // pop a scope from the stack
ast_symbol *scope_lookup(const char *symbol_name);
ast_func_declaration *get_scope_owning_function(); // get whose function's scope we are in
bool scope_symbol_declared_at_curr_level(const char *symbol_name);
void scope_declare_symbol(ast_symbol *symbol);

void print_symbol_table(scope *s);
