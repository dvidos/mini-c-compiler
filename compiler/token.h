#pragma once


enum token_type {
    TOK_COMMENT,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_COMMA,
    TOK_STRING_LITERAL,
    TOK_CHAR_LITERAL,
    TOK_OPEN_PARENTHESIS,
    TOK_CLOSE_PARENTHESIS,
    TOK_OPEN_BRACKET,
    TOK_CLOSE_BRACKET,
    TOK_OPEN_BLOCK,
    TOK_CLOSE_BLOCK,
    TOK_END_OF_STATEMENT,
    TOK_ASSIGNMENT,
    TOK_EQUALITY_CHECK,
    TOK_PLUS_SIGN,
    TOK_MINUS_SIGN,
    TOK_INCREMENT,
    TOK_DECREMENT,
    TOK_UNKNOWN,

    // keep this section synced with keywords
    TOK_KEYWORDS_START,

};

typedef struct token token;
struct token {
    struct token *next;
    enum token_type type;
    char *value;
    int entry;  // if specific keyword or data type
};

token *create_token(enum token_type type, char *value);

void init_tokens();
void add_token(token *token);
void print_token(token *token);
void print_tokens();
