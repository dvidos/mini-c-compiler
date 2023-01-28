#include <stddef.h>
#include "symbol.h"
#include "ast_node.h"

typedef struct scope {
    symbol *symbols_list_head;
    symbol *symbols_list_tail;

    func_declaration *scoped_func;
    struct scope *higher;
} scope;



void scope_entered(func_declaration *func); // creates a new scope on the stack
void scope_exited(); // pop a scope from the stack
symbol *scope_lookup(char *symbol_name);
func_declaration *get_function_in_scope(); // get whose function's scope we are in
bool scope_symbol_declared_at_curr_level(char *symbol_name);
void scope_declare_symbol(symbol *symbol);

void print_symbol_table(scope *s);
