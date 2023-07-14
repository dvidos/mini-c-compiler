#pragma once
#include "../lexer/token.h"
#include "token_iterator.h"
#include "../ast_module.h"
#include "../../utils/all.h"


void parse_file_level_element(mempool *mp, token_iterator *ti, ast_module *mod);
