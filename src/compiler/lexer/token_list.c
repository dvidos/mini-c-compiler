#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "token_list.h"

static void _add(token_list *list, token *token);
static void _print(token_list *list, char *prefix, bool unknown_only);
static bool _unknown_tokens_exist(token_list *list);


token_list *new_token_list() {
    token_list *list = malloc(sizeof(token_list));

    list->capacity = 10;
    list->tokens = malloc(list->capacity * sizeof(token *));
    list->length = 0;

    list->add = _add;
    list->print = _print;
    list->unknown_tokens_exist = _unknown_tokens_exist;

    return list;
}

static void _add(token_list *list, token *token) {
    if (list->length + 1 >= list->capacity) {
        list->capacity *= 2;
        list->tokens = realloc(list->tokens, sizeof(struct token *) * list->capacity);
    }

    list->tokens[list->length] = token;
    list->length++;
}

static void _print(token_list *list, char *prefix, bool unknown_only) {
    token *t;
    for (int i = 0; i < list->length; i++) {
        t = list->tokens[i];
        if (unknown_only && t->type != TOK_UNKNOWN)
            continue;
        
        char *name = token_type_name(t->type);
        if (t->value == NULL) {
            printf("%s%s:%d: %s\n", prefix, t->filename, t->line_no, name);
        } else {
            printf("%s%s:%d: %s \"%s\"\n", prefix, t->filename, t->line_no, name, t->value);
        }
    }
}

static bool _unknown_tokens_exist(token_list *list) {
    for (int i = 0; i < list->length; i++)
        if (list->tokens[i]->type == TOK_UNKNOWN)
            return true;
    return false;
}
