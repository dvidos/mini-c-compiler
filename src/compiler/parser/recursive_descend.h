#pragma once

#include "../lexer/token.h"
#include "../ast.h"
#include "../../utils/data_structs.h"


void parse_file_using_recursive_descend___deprecated();

ast_module_node *parse_file_tokens_using_recursive_descend(mempool *mp, llist *tokens);
