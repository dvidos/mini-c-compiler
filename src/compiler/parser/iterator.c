#include "stddef.h"
#include "stdio.h"
#include "stdarg.h"
#include "../../err_handler.h"
#include "../../run_info.h"
#include "../lexer/token_list.h"

// iteration for our parser
// to be used from both parsers:
// - recursive descend for file body
// - shunting yard for expressions


static token_list *list;
static int current_index;
static int accepted_index;

void init_token_iterator(token_list *l) {
    list = l;
    current_index = 0;
    accepted_index = -1;
}

token *next() {
    return list->tokens[current_index];
}

token *lookahead(int times) {
    int i = current_index;
    while (times-- > 0 && i < list->length && list->tokens[i]->type != TOK_EOF)
        i++;
    return list->tokens[i];
}

// returns the type of the next token, without advancing
bool next_is(token_type type) {
    if (current_index >= list->length)
        return type == TOK_EOF;
    return list->tokens[current_index]->type == type;
}

bool lookahead_is(int times, token_type type) {
    token *t = lookahead(times);
    if (t == NULL)
        return type == TOK_EOF;
    return t->type == type;
}

// advances to the next token
void consume() {
    if (current_index < list->length && list->tokens[current_index]->type != TOK_EOF) {
        if (run_info->options->verbose) {
            if (list->tokens[current_index]->value == NULL)
                printf("  Parsing AST, at line %d, consumed %s\n", list->tokens[current_index]->line_no, token_type_name(list->tokens[current_index]->type));
            else 
                printf("  Parsing AST, at line %d, consumed %s: %s\n", list->tokens[current_index]->line_no, token_type_name(list->tokens[current_index]->type), list->tokens[current_index]->value);
        }
        accepted_index = current_index;
        current_index++;
    }
}

// see if next token is type and accept (consume) if so.
bool accept(token_type type) {
    if (next_is(type)) {
        consume();
        return true;
    }
    return false;
}

// returns the last consumed token
token *accepted() {
    return list->tokens[accepted_index];
}

// verifies next token is of specified type, otherwise fail
bool expect(token_type type) {
    if (!next_is(type)) {
        error_at(
            list->tokens[current_index]->filename,
            list->tokens[current_index]->line_no,
            "was expecting \"%s\", but got \"%s\"", 
            token_type_name(type), 
            token_type_name(list->tokens[current_index]->type)
        );
        return false;
    }
    
    consume();
    return true;
}

