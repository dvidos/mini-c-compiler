#pragma once

#include "../declaration.h"

expression *parse_expression_using_shunting_yard(mempool *mp, token_iterator *ti);

