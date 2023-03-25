#pragma once

#include <stdbool.h>


typedef enum token_type {
    TOK_COMMENT,
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_NUMERIC_LITERAL,
    TOK_STRING_LITERAL,
    TOK_CHAR_LITERAL,

    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_BLOCK_START,
    TOK_BLOCK_END,

    TOK_COMMA,
    TOK_SEMICOLON,
    TOK_EQUAL_SIGN,
    TOK_DBL_EQUAL_SIGN,
    TOK_PLUS_SIGN,
    TOK_DBL_PLUS,
    TOK_MINUS_SIGN,
    TOK_DBL_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_AMPERSAND,
    TOK_DBL_AMPERSAND,
    TOK_PIPE,
    TOK_DBL_PIPE,
    TOK_LESS_THAN,
    TOK_LESS_EQUAL,
    TOK_DBL_LESS_THAN,
    TOK_LARGER_EQUAL,
    TOK_LARGER_THAN,
    TOK_DBL_GRATER_THAN,

    TOK_EXCLAMANTION,
    TOK_PERCENT,
    TOK_CARET,
    TOK_TILDE,
    TOK_QUESTION_MARK,
    TOK_COLON,
    TOK_DOT,
    TOK_ARROW,
    TOK_EXCLAM_EQUAL,
    TOK_ADD_ASSIGN,
    TOK_SUB_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,
    TOK_MOD_ASSIGN,
    TOK_RSH_ASSIGN,
    TOK_LSH_ASSIGN,
    TOK_AND_ASSIGN,
    TOK_OR_ASSIGN,
    TOK_XOR_ASSIGN,


    // keep this section synced with keywords
    TOK___KEYWORDS_START___,
    TOK_RETURN = TOK___KEYWORDS_START___,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_CONTINUE,
    TOK_BREAK,
    TOK_INT_KEYWORD,
    TOK_FLOAT,
    TOK_CHAR_KEYWORD,
    TOK_VOID,
    TOK_BOOL,
    TOK_TRUE,
    TOK_FALSE,

    // unknown token, e.g. something we don't understand.
    TOK_UNKNOWN,
} token_type;

typedef struct token token;
struct token {
    token_type type;
    char *value;
    int entry;  // if specific keyword or data type

    // helping with troubleshooting
    char *filename;
    int line_no;
};

token *create_token(token_type type, char *value, char *filename, int line_no);
char *token_type_name(token_type type);

