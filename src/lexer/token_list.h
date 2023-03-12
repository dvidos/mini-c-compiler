#pragma once
#include "token.h"

typedef struct token_list {
    token **tokens;
    int length;
    int capacity;

    void (*add)(struct token_list *list, token *token);
    void (*print)(struct token_list *list, char *prefix, bool unknown_only);
    bool (*unknown_tokens_exist)(struct token_list *list);
} token_list;

token_list *new_token_list();

