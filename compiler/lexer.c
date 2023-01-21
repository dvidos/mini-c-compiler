#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "defs.h"
#include "lexer.h"
#include "token.h"


#define is_digit(c)               ((c) >= '0' && (c) <= '9')
#define is_letter(c)              (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define is_underscore(c)          ((c) == '_')
#define is_newline(c)             ((c) == '\n')
#define is_whitespace(c)          ((c) == ' ' || (c) == '\t' || (c) == '\r' || is_newline(c))

#define is_identifier_start(c)    (is_letter(c) || is_underscore(c))
#define is_string_quote(c)        ((c) == '"')
#define is_char_quote(c)          ((c) == '\'')
#define is_block_comment(c, c2)   ((c) == '/' && (c2) == '*')
#define is_line_comment(c, c2)    ((c) == '/' && (c2) == '/')




static int collector_len = 0;
static int collector_capacity = 0;
static char *collector_buffer = NULL;

void init_lexer() {
    collector_capacity = 64;
    collector_buffer = malloc(collector_capacity);
    collector_len = 0;
}

static void clear_collection() {
    collector_len = 0;
}

static void collect(char c) {
    if (collector_len + 1 >= collector_capacity) {
        collector_capacity *= 2;
        collector_buffer = realloc(collector_buffer, collector_capacity);
    }
    collector_buffer[collector_len++] = c;
    collector_buffer[collector_len  ] = '\0';
}


static void skip_whitespace(char **p, int *line_no) {
    while (is_whitespace(**p)) {
        if (is_newline(**p)) 
            (*line_no)++;
        (*p)++;
    }
}

static int parse_block_comment(char **p, int *line_no) {
    (*p) += 2; // skip the "/*" opening part
    char c = **p;
    char c2 = *((*p) + 1);
    skip_whitespace(p, line_no);
    while (!(c == '*' && c2 == '/')) {
        if (is_newline(c))
            (*line_no) += 1;
        
        collect(c);
        (*p)++;
        c = **p;
        c2 = *((*p) + 1);
    }
    (*p) += 2; // skip the "*/" closing part
}

static int parse_line_comment(char **p) {
    (*p) += 2; // skip the "//" opening part
    while (**p == ' ')
        (*p)++;
    while (!is_newline(**p)) {
        collect(**p);
        (*p)++;
    }
}

static int parse_identifier(char **p) {
    char c = **p;
    while (is_letter(c) || is_digit(c) || is_underscore(c)) {
        collect(c);
        (*p)++;
        c = **p;
    }
}

static int parse_string(char **p) {
    (*p)++; // skip starting quote
    char c = **p;
    while (!is_string_quote(c)) {
        collect(c);
        (*p)++;
        c = **p;
        // need to parse escaped sequences (\", \n, \a, etc)
    }
    (*p)++; // skip ending quote
}

static int parse_number(char **p) {
    char c = **p;
    while (is_digit(c)) {
        collect(c);
        (*p)++;
        c = **p;
    }
}

static int parse_char(char **p) {
    (*p)++; // skip starting quote
    // should handle the '\'' case
    collect(**p);
    (*p)++; // skip character
    (*p)++; // skip ending quote
}


#define check1(base_char, base_type)   \
    else if (c == base_char) {         \
        type = base_type;              \
        (*p) += 1;                     \
    }

#define check1_ext(base_char, ext_char, base_type, ext_type)   \
    else if (c == base_char) {                                 \
        if (c2 == ext_char) { type = ext_type;  (*p) += 2; }   \
        else                { type = base_type; (*p) += 1; }   \
    }

int parse_lexer_token_at_pointer(char **p, char *filename, int *line_no, struct token **token) {

    (*token) = NULL;
    skip_whitespace(p, line_no);
    if (**p == '\0')
        return DONE;
    
    char c = **p;
    char c2 = *(*p + 1);
    enum token_type type;
    clear_collection();

    if (is_block_comment(c, c2)) {
        parse_block_comment(p, line_no);
        type = TOK_COMMENT;
    } 
    else if (is_line_comment(c, c2)) {
        parse_line_comment(p);
        type = TOK_COMMENT;
    }
    else if (is_identifier_start(c)) {
        parse_identifier(p);
        type = TOK_IDENTIFIER;
    }
    else if (is_digit(c)) {
        parse_number(p);
        type = TOK_NUMERIC_LITERAL;
    }
    else if (is_string_quote(c)) {
        parse_string(p);
        type = TOK_STRING_LITERAL;
    }
    else if (is_char_quote(c)) {
        parse_char(p);
        type = TOK_CHAR_LITERAL;
    }
    check1(',', TOK_COMMA)
    check1(';', TOK_END_OF_STATEMENT)
    check1('(', TOK_LPAREN)
    check1(')', TOK_RPAREN)
    check1('[', TOK_OPEN_BRACKET)
    check1(']', TOK_CLOSE_BRACKET)
    check1('{', TOK_BLOCK_START)
    check1('}', TOK_BLOCK_END)

    check1_ext('=', '=', TOK_ASSIGNMENT, TOK_EQUALITY_CHECK)
    check1_ext('!', '=', TOK_LOGICAL_NOT, TOK_NOT_EQUAL)
    check1_ext('<', '=', TOK_LESS_THAN, TOK_LESS_EQUAL)
    check1_ext('>', '=', TOK_LARGER_THAN, TOK_LARGER_EQUAL)
    check1_ext('+', '+', TOK_PLUS_SIGN, TOK_INCREMENT)
    check1_ext('-', '-', TOK_MINUS_SIGN, TOK_DECREMENT)
    check1_ext('|', '|', TOK_BITWISE_OR, TOK_LOGICAL_OR)
    check1_ext('&', '&', TOK_BITWISE_AND, TOK_LOGICAL_AND)

    check1('*', TOK_STAR)
    check1('/', TOK_SLASH)
    check1('^', TOK_CARET)
    check1('%', TOK_PERCENT)
    check1('~', TOK_TILDE)

    else {
        collect(c);
        type = TOK_UNKNOWN;
        (*p)++;
    }
    
    (*token) = create_token(
        type, 
        (collector_len == 0 ? NULL : strdup(collector_buffer)),
        filename, *line_no);
    return SUCCESS;
}

