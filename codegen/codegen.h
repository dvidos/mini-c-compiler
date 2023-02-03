#pragma once
#include "../declaration.h"
#include "../ast.h"


struct function_code_generation_info {
    func_declaration *decl;
    // a way to track which registers we use?
    // a way to track arguments and local variables, and their relation to BP
};
extern struct function_code_generation_info current_function_generated;



enum generation_type {
    GT_RETURNED_VALUE,  // result in AX or r0
    GT_BRANCH_DECISION, // result in BX or r1, for comparison
    GT_GENERIC,         // nothing specific
};

void generate_expression_code(expression *expr, enum generation_type target);
void generate_statement_code(statement *stmt);
void generate_function_code(func_declaration *func);
void generate_module_code(ast_module_node *module);

