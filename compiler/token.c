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
    "while",
    "continue",
    "break",
    "int",
    "char",
};

token *create_token(token_type type, char *value, char *filename, int line_no) {

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
    t->filename = filename;
    t->line_no = line_no;
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

char *token_type_name(enum token_type type) {
    switch (type) {
        case TOK_COMMENT: return "comment";
        case TOK_EOF: return "eof";
        case TOK_IDENTIFIER: return "identifier";
        case TOK_NUMBER: return "number";
        case TOK_COMMA: return ",";
        case TOK_STRING_LITERAL: return "string_literal";
        case TOK_CHAR_LITERAL: return "char_literal";
        case TOK_LPAREN: return "(";
        case TOK_RPAREN: return ")";
        case TOK_OPEN_BRACKET: return "[";
        case TOK_CLOSE_BRACKET: return "]";
        case TOK_BLOCK_START: return "{";
        case TOK_BLOCK_END: return "}";
        case TOK_END_OF_STATEMENT: return ";";
        case TOK_ASSIGNMENT: return "=";
        case TOK_EQUALITY_CHECK: return "==";
        case TOK_PLUS_SIGN: return "+";
        case TOK_MINUS_SIGN: return "-";
        case TOK_STAR: return "*";
        case TOK_SLASH: return "/";
        case TOK_AMPBERSAND: return "&";
        case TOK_PERCENT: return "%";
        case TOK_CARET: return "^";
        case TOK_QUESTION_MARK: return "?";
        case TOK_COLON: return ":";
        case TOK_DOT: return ".";
        case TOK_INCREMENT: return "++";
        case TOK_DECREMENT: return "--";
        case TOK_LESS_EQUAL: return "<=";
        case TOK_LESS_THAN: return "<";
        case TOK_LARGER_EQUAL: return ">=";
        case TOK_LARGER_THAN: return ">";
        case TOK_NOT_EQUAL: return "!=";
        case TOK_BOOLEAN_NOT: return "!";
        case TOK_LOGICAL_AND: return "&&";
        case TOK_BITWISE_AND: return "&";
        case TOK_LOGICAL_OR: return "||";
        case TOK_BITWISE_OR: return "|";
        case TOK_RETURN: return "return";
        case TOK_IF: return "if";
        case TOK_ELSE: return "else";
        case TOK_INT: return "int";
        case TOK_CHAR: return "char";
        case TOK_UNKNOWN: return "*** unknown ***";
        default: return "*** UN-NAMED ***";
    }
}

void print_token(token *token) {
    char *name = token_type_name(token->type);
    if (token->value == NULL) {
        printf("  %s\n", name);
    } else {
        printf("  %s: \"%s\"\n", name, token->value);
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

token *get_first_token() {
    return tokens_head;
}

