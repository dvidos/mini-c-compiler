#pragma once

#include <stdbool.h>


typedef enum token_type {
    TOK_COMMENT,
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_NUMERIC_LITERAL,
    TOK_COMMA,
    TOK_STRING_LITERAL,
    TOK_CHAR_LITERAL,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_OPEN_BRACKET,
    TOK_CLOSE_BRACKET,
    TOK_BLOCK_START,
    TOK_BLOCK_END,
    TOK_END_OF_STATEMENT,
    TOK_ASSIGNMENT,
    TOK_EQUALITY_CHECK,
    TOK_PLUS_SIGN,
    TOK_MINUS_SIGN,
    TOK_STAR,
    TOK_SLASH,
    TOK_AMPERSAND,
    TOK_PERCENT,
    TOK_CARET,
    TOK_TILDE,
    TOK_QUESTION_MARK,
    TOK_COLON,
    TOK_DOT,
    TOK_INCREMENT,
    TOK_DECREMENT,
    TOK_LESS_EQUAL,
    TOK_LESS_THAN,
    TOK_LARGER_EQUAL,
    TOK_LARGER_THAN,
    TOK_NOT_EQUAL,
    TOK_LOGICAL_NOT,
    TOK_LOGICAL_AND,
    TOK_BITWISE_AND,
    TOK_LOGICAL_OR,
    TOK_BITWISE_OR,


    // keep this section synced with keywords
    TOK_KEYWORDS_START,
    TOK_RETURN = TOK_KEYWORDS_START,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_CONTINUE,
    TOK_BREAK,
    TOK_INT_KEYWORD,
    TOK_CHAR_KEYWORD,
    TOK_VOID,

    // unknown token, e.g. something we don't understand.
    TOK_UNKNOWN,
} token_type;

typedef struct token token;
struct token {
    struct token *next;
    token_type type;
    char *value;
    int entry;  // if specific keyword or data type

    // helping with troubleshooting
    char *filename;
    int line_no;
};

token *create_token(token_type type, char *value, char *filename, int line_no);

void init_tokens();
void add_token(token *token);
void print_token(token *token, char *prefix);
char *token_type_name(token_type type);
void print_tokens(char *prefix);
int count_tokens();
bool unknown_tokens_exist();
token *get_first_token();
