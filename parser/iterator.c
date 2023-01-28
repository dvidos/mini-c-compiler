#include "stddef.h"
#include "stdio.h"
#include "stdarg.h"
#include "../error.h"
#include "../options.h"
#include "../lexer/token.h"

// iteration for our parser
// to be used from both parsers:
// - recursive descend for file body
// - shunting yard for expressions


static token *current_token;
static token *accepted_token;

void init_token_iterator(token *first_token) {
    current_token = first_token;
    accepted_token = NULL;
}

token *next() {
    return current_token;
}

token *lookahead(int times) {
    token *ahead = current_token;
    while (times-- > 0 && ahead != NULL && ahead->type != TOK_EOF)
        ahead = ahead->next;
    return ahead;
}

// returns the type of the next token, without advancing
bool next_is(token_type type) {
    if (current_token == NULL)
        return type == TOK_EOF;
    return current_token->type == type;
}

bool lookahead_is(int times, token_type type) {
    token *t = lookahead(times);
    if (t == NULL)
        return type == TOK_EOF;
    return t->type == type;
}

// advances to the next token
void consume() {

    if (current_token != NULL && current_token->type != TOK_EOF) {
        if (options.verbose) {
            if (current_token->value == NULL)
                printf("at line %d, consumed %s\n", current_token->line_no, token_type_name(current_token->type));
            else 
                printf("at line %d, consumed %s: %s\n", current_token->line_no, token_type_name(current_token->type), current_token->value);
        }
        accepted_token = current_token;
        current_token = current_token->next;
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
    return accepted_token;
}

// verifies next token is of specified type, otherwise fail
bool expect(token_type type) {
    if (!next_is(type)) {
        error(
            current_token->filename,
            current_token->line_no,
            "was expecting \"%s\", but got \"%s\"", 
            token_type_name(type), 
            token_type_name(current_token->type)
        );
        return false;
    }
    
    consume();
    return true;
}

