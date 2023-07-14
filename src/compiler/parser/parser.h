#include "../../utils/all.h"
#include "../ast/all.h"


ast_module *parse_file_tokens_into_ast(mempool *mp, list *tokens);

#ifdef INCLUDE_UNIT_TESTS
void parser_unit_tests();
#endif
