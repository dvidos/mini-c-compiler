#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "../../err_handler.h"
#include "../../run_info.h"
#include "token_iterator.h"


typedef struct token_iterator_private_data {
    list *tokens;
    iterator *iter;
    token *upcoming; // "next" / peek
    token *accepted; // "past"
} token_iterator_private_data;

static token *token_iterator_next(token_iterator *ti);
static token *token_iterator_lookahead(token_iterator *ti, int times);
static bool token_iterator_next_is(token_iterator *ti, token_type type);
static bool token_iterator_lookahead_is(token_iterator *ti, int times, token_type type);
static void token_iterator_consume(token_iterator *ti);
static bool token_iterator_accept(token_iterator *ti, token_type type);
static token *token_iterator_accepted(token_iterator *ti);
static bool token_iterator_expect(token_iterator *ti, token_type type);

token_iterator *new_token_iterator(mempool *mp, list *tokens) {
    token_iterator_private_data *data = mpalloc(mp, token_iterator_private_data);
    data->tokens = tokens;
    data->iter = list_create_iterator(data->tokens, mp);
    data->accepted = NULL;
    data->upcoming = data->iter->reset(data->iter);

    token_iterator *ti = mpalloc(mp, token_iterator);
    ti->next = token_iterator_next;
    ti->lookahead = token_iterator_lookahead;
    ti->next_is = token_iterator_next_is;
    ti->lookahead_is = token_iterator_lookahead_is;
    ti->consume = token_iterator_consume;
    ti->accept = token_iterator_accept;
    ti->accepted = token_iterator_accepted;
    ti->expect = token_iterator_expect;
    ti->priv_data = data;

    return ti;
}

static token *token_iterator_next(token_iterator *ti) {
    token_iterator_private_data *data = (token_iterator_private_data *)ti->priv_data;

    // same as peek the next one
    return data->upcoming;
}

static token *token_iterator_lookahead(token_iterator *ti, int times) {
    token_iterator_private_data *data = (token_iterator_private_data *)ti->priv_data;

    // peek ahead
    return (token *)data->iter->lookahead(data->iter, times);
}

static bool token_iterator_next_is(token_iterator *ti, token_type type) {
    token_iterator_private_data *data = (token_iterator_private_data *)ti->priv_data;

    return (data->upcoming != NULL && data->upcoming->type == type);
}

static bool token_iterator_lookahead_is(token_iterator *ti, int times, token_type type) {
    token_iterator_private_data *data = (token_iterator_private_data *)ti->priv_data;

    token *ahead = data->iter->lookahead(data->iter, times);
    return (ahead != NULL && ahead->type == type);
}

static void token_iterator_consume(token_iterator *ti) {
    token_iterator_private_data *data = (token_iterator_private_data *)ti->priv_data;

    if (data->accepted != NULL && data->accepted->type == TOK_EOF)
        return;
    
    data->accepted = data->upcoming;
    data->upcoming = (token *)data->iter->next(data->iter);
}

static bool token_iterator_accept(token_iterator *ti, token_type type) {
    token_iterator_private_data *data = (token_iterator_private_data *)ti->priv_data;

    if (token_iterator_next_is(ti, type)) {
        token_iterator_consume(ti);
        return true;
    }

    return false;
}

static token *token_iterator_accepted(token_iterator *ti) {
    token_iterator_private_data *data = (token_iterator_private_data *)ti->priv_data;

    // peek back.
    return data->accepted;
}

static bool token_iterator_expect(token_iterator *ti, token_type type) {
    token_iterator_private_data *data = (token_iterator_private_data *)ti->priv_data;

    if (!token_iterator_next_is(ti, type)) {
        error_at(data->accepted->filename, data->accepted->line_no,
            "Expecting token \"%s\", but got \"%s\"", 
            token_type_name(type), 
            token_type_name(ti->next(ti)->type)
        );
        return false;
    }
    
    token_iterator_consume(ti);
    return true;
}

