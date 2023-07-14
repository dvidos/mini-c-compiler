#pragma once
#include "token.h"

list *lexer_parse_source_code_into_tokens(mempool *mp, str *filename, str *source_code);
bool lexer_check_tokens(list *tokens, str *filename);