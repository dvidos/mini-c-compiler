#include <stdlib.h>
#include "token.h"


token *tokens_head;
token *tokens_tail;

token *create_token(enum token_type type, char *value) {
    token *t = malloc(sizeof(token));

    t->type = type;
    if (atom_len() == 0)
        t->value = NULL;
    else {
        t->value = malloc(atom_len() + 1);
        strcpy(t->value, value);
    }

    return t;
}

void init_tokens() {
    tokens_head = NULL;
    tokens_tail = NULL;
}

void add_token(token *token) {
    if (tokens_head == NULL) {
        tokens_head = token;
        tokens_tail = token;
    } else {
        tokens_tail->next = token;
        tokens_tail = token;
    }
    token->next = NULL;
}

void print_token(token *token) {
    char *name;
    switch (token->type) {
        case TOK_COMMENT: name = "COMMENT"; break;
        case TOK_IDENTIFIER: name = "IDENTIFIER"; break;
        case TOK_NUMBER: name = "NUMBER"; break;
        case TOK_COMMA: name = "COMMA"; break;
        case TOK_STRING_LITERAL: name = "STRING_LITERAL"; break;
        case TOK_CHAR_LITERAL: name = "CHAR_LITERAL"; break;
        case TOK_OPEN_PARENTHESIS: name = "OPEN_PARENTHESIS"; break;
        case TOK_CLOSE_PARENTHESIS: name = "CLOSE_PARENTHESIS"; break;
        case TOK_OPEN_BRACKET: name = "OPEN_BRACKET"; break;
        case TOK_CLOSE_BRACKET: name = "CLOSE_BRACKET"; break;
        case TOK_OPEN_BLOCK: name = "OPEN_BLOCK"; break;
        case TOK_CLOSE_BLOCK: name = "CLOSE_BLOCK"; break;
        case TOK_END_OF_STATEMENT: name = "END_OF_STATEMENT"; break;
        case TOK_ASSIGNMENT: name = "ASSIGNMENT"; break;
        case TOK_EQUALITY_CHECK: name = "EQUALITY_CHECK"; break;
        case TOK_PLUS_SIGN: name = "PLUS_SIGN"; break;
        case TOK_MINUS_SIGN: name = "MINUS_SIGN"; break;
        case TOK_INCREMENT: name = "INCREMENT"; break;
        case TOK_DECREMENT: name = "DECREMENT"; break;
        case TOK_UNKNOWN: name = "UNKNOWN"; break;
        default: name = "???"; break;
    }

    if (token->value == NULL) {
        printf("%s\n", name);
    } else {
        printf("%s \"%s\"\n", name, token->value);
    }
}

void print_tokens() {
    token *p = tokens_head;
    while (p != NULL) {
        print_token(p);
        p = p->next;
    }
}
