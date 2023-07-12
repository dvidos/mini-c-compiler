#pragma once

#include "../ast_declaration.h"

ast_expression *parse_expression_using_shunting_yard(mempool *mp, token_iterator *ti);

