#pragma once

#include "stddef.h"
#include "stdio.h"
#include "stdarg.h"
#include "../lexer/token_list.h"
#include "../lexer/token.h"
#include "../../utils/all.h"


typedef struct token_iterator token_iterator;

struct token_iterator {
    // returns the next (aka current) token, as opposed to accepted()
    token *(*next)(token_iterator *ti);

    // allows examination of subsequent tokens
    token *(*lookahead)(token_iterator *ti, int times);

    // checks the type of the next token, without advancing
    bool (*next_is)(token_iterator *ti, token_type type);

    // checks the type of subsequent tokens, without advancing
    bool (*lookahead_is)(token_iterator *ti, int times, token_type type);

    // advances to the next token
    void (*consume)(token_iterator *ti);

    // see if next token is type and accept (consume) if so.
    bool (*accept)(token_iterator *ti, token_type type);

    // returns the last consumed token
    token *(*accepted)(token_iterator *ti);

    // verifies next token is of specified type, otherwise fail
    bool (*expect)(token_iterator *ti, token_type type);

    // do not look
    void *priv_data;
};

token_iterator *new_token_iterator(mempool *mp, list *tokens);

