#pragma once

#include "token.h"

int parse_lexer_token_at_pointer(char **p, char *filename, int *line_no, token **token);

