#pragma once

#include "../lexer/token.h"
#include "../ast_module.h"
#include "../../utils/data_structs.h"


ast_module *parse_file_tokens_using_recursive_descend(mempool *mp, llist *tokens);
