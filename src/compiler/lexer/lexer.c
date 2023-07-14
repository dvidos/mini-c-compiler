#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../../err_handler.h"
#include "../../run_info.h"
#include "../../utils/all.h"
#include "lexer.h"
#include "token.h"


#define is_digit(c)               ((c) >= '0' && (c) <= '9')
#define is_hex_digit_or_sign(c)   (((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F') || (c) == 'x' || (c) == 'X')
#define is_letter(c)              (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define is_underscore(c)          ((c) == '_')
#define is_newline(c)             ((c) == '\n')
#define is_whitespace(c)          ((c) == ' ' || (c) == '\t' || (c) == '\r' || is_newline(c))

#define is_identifier_start(c)    (is_letter(c) || is_underscore(c))
#define is_string_quote(c)        ((c) == '"')
#define is_char_quote(c)          ((c) == '\'')
#define is_block_comment(c, c2)   ((c) == '/' && (c2) == '*')
#define is_line_comment(c, c2)    ((c) == '/' && (c2) == '/')


static void skip_whitespace(char **p, int *line_no) {
    while (is_whitespace(**p)) {
        if (is_newline(**p)) 
            (*line_no)++;
        (*p)++;
    }
}

static int parse_block_comment(str *buffer, char **p, int *line_no) {
    (*p) += 2; // skip the "/*" opening part
    char c = **p;
    char c2 = *((*p) + 1);
    while (!(c == '*' && c2 == '/') && c != '\0') {
        if (is_newline(c))
            (*line_no) += 1;
        
        str_catc(buffer, c);
        (*p)++;
        c = **p;
        c2 = *((*p) + 1);
    }
    (*p) += 2; // skip the "*/" closing part
}

static int parse_line_comment(str *buffer, char **p) {
    (*p) += 2; // skip the "//" opening part
    while (**p == ' ')
        (*p)++;
    while (!is_newline(**p)) {
        str_catc(buffer, **p);
        (*p)++;
    }
}

static int parse_identifier(str *buffer, char **p) {
    str_clear(buffer);
    char c = **p;
    while (is_letter(c) || is_digit(c) || is_underscore(c)) {
        str_catc(buffer, c);
        (*p)++;
        c = **p;
    }
}

static char backslash_escaped_char(char c) {
    switch (c) {
        case '\\': return '\\';
        case '\'': return '\'';
        case '"': return '"';
        case 'r': return '\r';
        case 'n': return '\n';
        case 't': return '\t';
        case '0': return '\0';
    }
    return c;
}

static int parse_string(str *buffer, char **p) {
    str_clear(buffer);
    (*p)++; // skip starting quote
    char c = **p;
    while (!is_string_quote(c)) {
        if (c == '\\') {
            (*p)++;
            c = backslash_escaped_char(**p);
        }
        str_catc(buffer, c);
        (*p)++;
        c = **p;
    }
    (*p)++; // skip ending quote
}

static int parse_number(str *buffer, char **p) {
    str_clear(buffer);
    char c = **p;
    while (is_digit(c) || is_hex_digit_or_sign(c)) {
        str_catc(buffer, c);
        (*p)++;
        c = **p;
    }
}

static int parse_char(str *buffer, char **p) {
    str_clear(buffer);
    (*p)++; // skip starting quote
    char c = **p;
    if (c == '\\') {
        (*p)++;
        c = backslash_escaped_char(**p);
    }
    str_catc(buffer, c);
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

#define check2(char1, char2, target_type)   \
    else if (c == char1 && c2 == char2) {   \
        type = target_type;                    \
        (*p) += 2;                          \
    }

static token *parse_lexer_token_at_pointer(mempool *mp, str *buffer, char **p, const char *filename, int *line_no) {
    str_clear(buffer);

    skip_whitespace(p, line_no);
    if (**p == '\0')
        return NULL;
    
    char c = **p;
    char c2 = *(*p + 1);
    enum token_type type;

    if (is_block_comment(c, c2)) {
        parse_block_comment(buffer, p, line_no);
        type = TOK_COMMENT;
    } 
    else if (is_line_comment(c, c2)) {
        parse_line_comment(buffer, p);
        type = TOK_COMMENT;
    }
    else if (is_identifier_start(c)) {
        parse_identifier(buffer, p);
        type = TOK_IDENTIFIER;
    }
    else if (is_digit(c)) {
        parse_number(buffer, p);
        type = TOK_NUMERIC_LITERAL;
    }
    else if (is_string_quote(c)) {
        parse_string(buffer, p);
        type = TOK_STRING_LITERAL;
    }
    else if (is_char_quote(c)) {
        parse_char(buffer, p);
        type = TOK_CHAR_LITERAL;
    }
    check1(',', TOK_COMMA)
    check1(';', TOK_SEMICOLON)
    check1('(', TOK_LPAREN)
    check1(')', TOK_RPAREN)
    check1('[', TOK_LBRACKET)
    check1(']', TOK_RBRACKET)
    check1('{', TOK_BLOCK_START)
    check1('}', TOK_BLOCK_END)
    check1('.', TOK_DOT)
    check1('*', TOK_STAR)
    check1('/', TOK_SLASH)
    check1('^', TOK_CARET)
    check1('%', TOK_PERCENT)
    check1('~', TOK_TILDE)

    check2('-', '>', TOK_ARROW)
    check2('<', '<', TOK_DBL_LESS_THAN)
    check2('>', '>', TOK_DBL_GRATER_THAN)

    check1_ext('=', '=', TOK_EQUAL_SIGN, TOK_DBL_EQUAL_SIGN)
    check1_ext('!', '=', TOK_EXCLAMANTION, TOK_EXCLAM_EQUAL)
    check1_ext('<', '=', TOK_LESS_THAN, TOK_LESS_EQUAL)
    check1_ext('>', '=', TOK_LARGER_THAN, TOK_LARGER_EQUAL)
    check1_ext('+', '+', TOK_PLUS_SIGN, TOK_DBL_PLUS)
    check1_ext('-', '-', TOK_MINUS_SIGN, TOK_DBL_MINUS)
    check1_ext('|', '|', TOK_PIPE, TOK_DBL_PIPE)
    check1_ext('&', '&', TOK_AMPERSAND, TOK_DBL_AMPERSAND)

    else {
        str_catc(buffer, c);
        type = TOK_UNKNOWN;
        (*p)++;
    }
    
    char *value = NULL;
    if (type == TOK_STRING_LITERAL
        || type == TOK_CHAR_LITERAL
        || type == TOK_NUMERIC_LITERAL
        || type == TOK_IDENTIFIER
        || type == TOK_UNKNOWN) {
        value = strdup(str_charptr(buffer)); // can be a string of zero length
    }
    
    token *t = new_token(mp, type, value, filename, *line_no);
    skip_whitespace(p, line_no);

    return t;
}


llist *lexer_parse_source_code_into_tokens(mempool *mp, str *filename, str *source_code) {
    llist *tokens = new_llist(mp);
    const char *p = str_charptr(source_code);
    token *token = NULL;
    str *buffer = new_str(mp, NULL);
    int line_no = 1;
    const char *fn = str_charptr(filename);

    while (*p != '\0') {
        token = parse_lexer_token_at_pointer(mp, buffer, (char **)&p, fn, &line_no);
        if (errors_count)
            return NULL;
        
        if (token == NULL)
            break;
        if (token->type == TOK_COMMENT)
            continue;
        
        llist_add(tokens, token);
    }

    // one final token, to allow us to always peek at the subsequent token
    llist_add(tokens, new_token(mp, TOK_EOF, NULL, fn, 999999));

    return tokens;
}

static void lexer_print_tokens(llist *tokens, char *prefix, bool unknown_only) {
    int line_no = -1;
    for_list(tokens, token, t) {
        if (unknown_only && t->type != TOK_UNKNOWN)
            continue;
        if (t->type == TOK_EOF)
            continue;
        
        if (t->line_no != line_no) {
            if (t->line_no > 1)
                printf("\n");
            printf("%s%d:", prefix, t->line_no);
            line_no = t->line_no;
        }

        char *name = token_type_name(t->type);
        if (t->value == NULL)
            printf(" %s", name);
        else
            printf(" %s \"%s\"", name, t->value);
    }
    printf("\n");
}


bool lexer_check_tokens(llist *tokens, str *filename) {
    // verify if unknown tokens exist
    for_list(tokens, token, t) {
        if (t->type == TOK_UNKNOWN) {
            error_at(str_charptr(filename), 0, "Unkown tokens found:");
            lexer_print_tokens(tokens, "  ", true);
            return false;
        }
    }

    // print tokens if requested
    if (run_info->options->verbose)
        lexer_print_tokens(tokens, "  ", false);

    return true;
}
