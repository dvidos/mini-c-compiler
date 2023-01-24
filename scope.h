#include <stddef.h>
#include "symbol.h"

typedef struct scope {
    symbol *symbols_list_head;
    symbol *symbols_list_tail;

    struct scope *higher;
} scope;



void scope_entered(); // creates a new scope on the stack
void scope_exited(); // pop a scope from the stack
symbol *scope_lookup(char *symbol_name);
bool scope_declared_at_curr_level(char *symbol_name);
void scope_declare_symbol(symbol *symbol);


