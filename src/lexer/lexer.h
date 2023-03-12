#pragma once
#include "token.h"

void init_lexer();
void parse_lexer_token_at_pointer(char **p, char *filename, int *line_no, token **token);

