#include <stddef.h>
#include <stdlib.h>
#include "../lexer/token.h"
#include "ast_operator.h"
#include "ast_data_type.h"
#include "ast_statement.h"
#include "ast_function.h"
#include "ast_variable.h"


ast_variable *new_ast_variable(mempool *mp, ast_data_type *data_type, const char* var_name, token *token) {
    ast_variable *v = mpalloc(mp, ast_variable);
    v->data_type = data_type;
    v->var_name = var_name;
    v->token = token;
    v->next = NULL;
    v->mempool = mp;
    return v;
}

