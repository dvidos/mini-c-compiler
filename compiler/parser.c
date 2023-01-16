#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "defs.h"
#include "atom.h"
#include "parser.h"


#define is_digit(c)         ((c) >= '0' && (c) <= '9')
#define is_letter(c)        (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define is_whitespace(c)    ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')


int parse_token_at_pointer(char **p, struct token *dest) {
    char c, cnext;
    enum token_type type;
    clear_atom();

    // skip whitespace, grab first character
    while (is_whitespace(**p))
        (*p)++;
    if (**p == 0)
        return DONE;
    
    c = **p;
    cnext = *(*p + 1);
    if (c == '/' && cnext == '/') {
        // parse comment till EOL
        (*p)++; // skip first slash
        (*p)++; // skip second slash
        while (is_whitespace(**p))
            (*p)++;
        c = **p;
        while (c != '\n') {
            extend_atom(c);
            (*p)++;
            c = **p;
        }
        type = TOK_COMMENT;
        
    } else if (is_letter(c) || c == '_') {
        // parse keyword or identifier
        while (is_letter(c) || is_digit(c) || c == '_') {
            extend_atom(c);
            (*p)++;
            c = **p;
        }
        type = TOK_IDENTIFIER;

    } else if (c >= '0' && c <= '9') {
        // parse number
        while (is_digit(c)) {
            extend_atom(c);
            (*p)++;
            c = **p;
        }
        type = TOK_NUMBER;

    } else if (c == ';') {
        // comma
        (*p)++;
        type = TOK_END_OF_STATEMENT;

    } else if (c == ',') {
        // comma
        (*p)++;
        type = TOK_COMMA;

    } else if (c == '"') {
        // parse string
        (*p)++; // skip starting quote
        c = **p;
        while (c != '"') {
            extend_atom(c);
            (*p)++;
            c = **p;
        }
        (*p)++; // skip ending quote
        type = TOK_STRING_LITERAL;
        
    } else if (c == '\'') {
        // parse char
        (*p)++; // skip starting quote
        c = **p;
        (*p)++; // skip character
        (*p)++; // skip ending quote
        type = TOK_CHAR_LITERAL;

    } else if (c == '(') {
        (*p)++; // skip one
        type = TOK_OPEN_PARENTHESIS;

    } else if (c == ')') {
        (*p)++; // skip one
        type = TOK_CLOSE_PARENTHESIS;

    } else if (c == '[') {
        (*p)++; // skip one
        type = TOK_OPEN_BRACKET;

    } else if (c == ']') {
        (*p)++; // skip one
        type = TOK_CLOSE_BRACKET;

    } else if (c == '{') {
        (*p)++; // skip one
        type = TOK_OPEN_BLOCK;

    } else if (c == '}') {
        (*p)++; // skip one
        type = TOK_CLOSE_BLOCK;

    } else if (c == '}') {
        (*p)++; // skip one
        type = TOK_CLOSE_BLOCK;

    } else if (c == '=') {
        (*p)++; // skip this
        if (**p == '=') {
            type = TOK_EQUALITY_CHECK;
            (*p)++;
        } else {
            type = TOK_ASSIGNMENT;
        }

    } else if (c == '+') {
        (*p)++; // skip this
        if (**p == '+') {
            type = TOK_INCREMENT;
            (*p)++;
        } else {
            type = TOK_PLUS_SIGN;
        }

    } else if (c == '-') {
        (*p)++; // skip this
        if (**p == '-') {
            type = TOK_DECREMENT;
            (*p)++;
        } else {
            type = TOK_MINUS_SIGN;
        }

    } else {
        (*p)++; // skip it.
        type = TOK_UNKNOWN;
    }

    dest->type = type;
    if (atom_len() == 0)
        dest->value = NULL;
    else {
        dest->value = malloc(atom_len() + 1);
        strcpy(dest->value, get_atom());
    }
    return SUCCESS;
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
