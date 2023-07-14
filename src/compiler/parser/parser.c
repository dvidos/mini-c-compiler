#include <string.h>
#include "parser.h"
#include "recursive_descend.h"
#include "token_iterator.h"
#include "../ast_operator.h"
#include "../../err_handler.h"
#include "../lexer/lexer.h"



ast_module *parse_file_tokens_into_ast(mempool *mp, list *tokens) {

    init_operators(); // make sure our lookup is populated

    ast_module *mod = new_ast_module(mp);
    token_iterator *ti = new_token_iterator(mp, tokens);

    while (!ti->next_is(ti, TOK_EOF) && errors_count == 0)
        parse_file_level_element(mp, ti, mod);

    return mod;
}


#ifdef INCLUDE_UNIT_TESTS
void parser_unit_tests() {
    mempool *mp = new_mempool();
    str *filename = new_str(mp, "file1.c");
    str *code;
    list *tokens;
    ast_module *ast;
    ast_statement *st;
    ast_func_declaration *fd;
    ast_var_declaration *vd;


    code = new_str(mp, "char *ptr;");
    tokens = lexer_parse_source_code_into_tokens(mp, filename, code);
    ast = parse_file_tokens_into_ast(mp, tokens);
    assert(ast != NULL);
    assert(list_length(ast->statements) == 1);
    assert(list_length(ast->functions) == 0);
    st = list_get(ast->statements, 0);
    assert(st != NULL);
    assert(st->stmt_type == ST_VAR_DECL);
    assert(st->decl != NULL);
    assert(strcmp(st->decl->var_name, "ptr") == 0);
    assert(st->decl->data_type != NULL);
    assert(st->decl->data_type->family == TF_POINTER);
    assert(st->decl->data_type->nested != NULL);
    assert(st->decl->data_type->nested->family == TF_CHAR);


    code = new_str(mp, "int a = 0x123;");
    tokens = lexer_parse_source_code_into_tokens(mp, filename, code);
    ast = parse_file_tokens_into_ast(mp, tokens);
    assert(ast != NULL);
    assert(list_length(ast->statements) == 1);
    assert(list_length(ast->functions) == 0);
    st = list_get(ast->statements, 0);
    assert(st->stmt_type == ST_VAR_DECL);
    assert(st->decl->data_type->family == TF_INT);
    assert(st->expr != NULL);
    assert(st->expr->op == OP_NUM_LITERAL);
    assert(st->expr->value.num == 0x123);


    code = new_str(mp, "int sum(int a, int b) { return a + b; }");
    tokens = lexer_parse_source_code_into_tokens(mp, filename, code);
    ast = parse_file_tokens_into_ast(mp, tokens);
    assert(ast != NULL);
    assert(list_length(ast->functions) == 1);

    fd = list_get(ast->functions, 0);
    assert(fd != NULL);
    assert(strcmp(fd->func_name, "sum") == 0);
    assert(fd->return_type != NULL);
    assert(fd->return_type->family == TF_INT);
    assert(fd->args_list != NULL);
    assert(fd->stmts_list != NULL);

    vd = fd->args_list;
    assert(vd->data_type != NULL);
    assert(vd->data_type->family == TF_INT);
    assert(strcmp(vd->var_name, "a") == 0);

    st = fd->stmts_list;
    assert(st->stmt_type == ST_RETURN);
    assert(st->expr != NULL);
    assert(st->expr->op == OP_ADD);
    assert(st->expr->arg1 != NULL);
    assert(st->expr->arg1->op == OP_SYMBOL_NAME);
    assert(strcmp(st->expr->arg1->value.str, "a") == 0);
    assert(st->expr->arg2 != NULL);
    assert(st->expr->arg2->op == OP_SYMBOL_NAME);
    assert(strcmp(st->expr->arg2->value.str, "b") == 0);
    
    // ideally we should test loop, break, continue, conditions etc
    // also, should test operator hierarchy, i.e. "result = x + y * z;"
    // finally, test calling a function from pointer of an array in a struct member

    mempool_release(mp);
}
#endif
