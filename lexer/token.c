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
    "float",
    "char",
    "void",
    "bool",
    "true",
    "false"
};

token *create_token(token_type type, char *value, char *filename, int line_no) {

    // see if identifier is a reserved word
    if (type == TOK_IDENTIFIER && value != NULL) {
        for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
            if (strcmp(value, keywords[i]) == 0) {
                type = (enum token_type)(TOK___KEYWORDS_START___ + i),
                value = NULL;
                break;
            }
        }
    }

    token *t = malloc(sizeof(token));
    t->type = type;
    if (value == NULL)
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
        case TOK_NUMERIC_LITERAL: return "numeric_literal";
        case TOK_COMMA: return ",";
        case TOK_STRING_LITERAL: return "string_literal";
        case TOK_CHAR_LITERAL: return "char_literal";
        case TOK_LPAREN: return "(";
        case TOK_RPAREN: return ")";
        case TOK_LBRACKET: return "[";
        case TOK_RBRACKET: return "]";
        case TOK_BLOCK_START: return "{";
        case TOK_BLOCK_END: return "}";
        case TOK_SEMICOLON: return ";";
        case TOK_EQUAL_SIGN: return "=";
        case TOK_DBL_EQUAL_SIGN: return "==";
        case TOK_PLUS_SIGN: return "+";
        case TOK_MINUS_SIGN: return "-";
        case TOK_STAR: return "*";
        case TOK_SLASH: return "/";
        case TOK_AMPERSAND: return "&";
        case TOK_PERCENT: return "%";
        case TOK_CARET: return "^";
        case TOK_QUESTION_MARK: return "?";
        case TOK_COLON: return ":";
        case TOK_DOT: return ".";
        case TOK_DBL_PLUS: return "++";
        case TOK_DBL_MINUS: return "--";
        case TOK_LESS_EQUAL: return "<=";
        case TOK_LESS_THAN: return "<";
        case TOK_LARGER_EQUAL: return ">=";
        case TOK_LARGER_THAN: return ">";
        case TOK_EXCLAM_EQUAL: return "!=";
        case TOK_EXCLAMANTION: return "!";
        case TOK_DBL_AMPERSAND: return "&&";
        case TOK_DBL_PIPE: return "||";
        case TOK_PIPE: return "|";

        case TOK_RETURN: return "return";
        case TOK_IF: return "if";
        case TOK_ELSE: return "else";
        case TOK_WHILE: return "while";
        case TOK_CONTINUE: return "continue";
        case TOK_BREAK: return "break";
        case TOK_INT_KEYWORD: return "int";
        case TOK_FLOAT: return "float";
        case TOK_CHAR_KEYWORD: return "char";
        case TOK_BOOL: return "bool";
        case TOK_VOID: return "void";
        case TOK_TRUE: return "true";
        case TOK_FALSE: return "false";
        case TOK_DBL_GRATER_THAN: return ">>";
        case TOK_DBL_LESS_THAN: return "<<";
        case TOK_UNKNOWN: return "*** unknown ***";
        default: return "*** UN-NAMED ***";
    }
}

void print_token(token *token, char *prefix, bool unknown_only) {
    char *name = token_type_name(token->type);

    if (unknown_only) {
      if ((strcmp(name, "*** unknown ***") != 0))
        return;
    } 
    
    if (token->value == NULL) {
        printf("%sline %d: %s\n", prefix, token->line_no, name);
    } else {
        printf("%sline %d: %s \"%s\"\n", prefix, token->line_no, name, token->value);
    }
}

void print_tokens(char *prefix, bool unknown_only) {
    token *p = tokens_head;
    while (p != NULL) {
        print_token(p, prefix, unknown_only);
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

