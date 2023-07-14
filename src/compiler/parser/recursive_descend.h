#pragma once

#include "../lexer/token.h"
#include "../ast_module.h"
#include "../../utils/all.h"


ast_module *parse_file_tokens_using_recursive_descend(mempool *mp, list *tokens);
