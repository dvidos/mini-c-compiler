#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "token.h"

token *tokens_head;
token *tokens_tail;

char *keywords[] = {
    "return",
    "if",
    "else",
};

token *create_token(enum token_type type, char *value) {

    // see if identifier is a reserved word
    if (type == TOK_IDENTIFIER && value != NULL) {
        for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
            if (strcmp(value, keywords[i]) == 0) {
                type = (enum token_type)(TOK_KEYWORDS_START + i),
                value = NULL;
                break;
            }
        }
    }

    token *t = malloc(sizeof(token));
    t->type = type;
    if (value == NULL || strlen(value) == 0)
        t->value = NULL;
    else {
        t->value = malloc(strlen(value) + 1);
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
        case TOK_LESS_EQUAL: name = "LESS_EQUAL"; break;
        case TOK_LESS_THAN: name = "LESS_THAN"; break;
        case TOK_LARGER_EQUAL: name = "LARGER_EQUAL"; break;
        case TOK_LARGER_THAN: name = "LARGER_THAN"; break;
        case TOK_NOT_EQUAL: name = "NOT_EQUAL"; break;
        case TOK_BOOLEAN_NOT: name = "BOOLEAN_NOT"; break;
        case TOK_LOGICAL_AND: name = "LOGICAL_AND"; break;
        case TOK_BITWISE_AND: name = "BITWISE_AND"; break;
        case TOK_LOGICAL_OR: name = "LOGICAL_OR"; break;
        case TOK_BITWISE_OR: name = "BITWISE_OR"; break;

        // keywods
        case TOK_RETURN: name = "RETURN"; break;
        case TOK_IF: name = "IF"; break;
        case TOK_ELSE: name = "ELSE"; break;

        // exceptions
        case TOK_UNKNOWN: name = "** UNKNOWN **"; break;
        default: name = "** NAME NOT GIVEN **"; break;
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

int count_tokens() {
    int i = 0;
    token *p = tokens_head;
    while (p != NULL) {
        i++;
        p = p->next;
    }
    return i;
}

bool unknown_tokens_exist() {
    token *p = tokens_head;
    while (p != NULL) {
        if (p->type == TOK_UNKNOWN)
            return true;
        p = p->next;
    }
    return false;
}