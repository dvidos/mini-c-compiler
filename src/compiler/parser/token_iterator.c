#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include "../../err_handler.h"
#include "../../run_info.h"
#include "../lexer/token_list.h"
#include "token_iterator.h"


// --------------------------------
// older code
// --------------------------------


// iteration for our parser
// to be used from both parsers:
// - recursive descend for file body
// - shunting yard for expressions


static token_list *list;
static int current_index;
static int accepted_index;

void init_token_iterator(token_list *l) {
    list = l;
    current_index = 0;
    accepted_index = -1;
}

token *next() {
    return list->tokens[current_index];
}

token *lookahead(int times) {
    int i = current_index;
    while (times-- > 0 && i < list->length && list->tokens[i]->type != TOK_EOF)
        i++;
    return list->tokens[i];
}

// returns the type of the next token, without advancing
bool next_is(token_type type) {
    if (current_index >= list->length)
        return type == TOK_EOF;
    return list->tokens[current_index]->type == type;
}

bool lookahead_is(int times, token_type type) {
    token *t = lookahead(times);
    if (t == NULL)
        return type == TOK_EOF;
    return t->type == type;
}

// advances to the next token
void consume() {
    if (current_index < list->length && list->tokens[current_index]->type != TOK_EOF) {
        if (run_info->options->verbose) {
            if (list->tokens[current_index]->value == NULL)
                printf("  Parsing AST, at line %d, consumed %s\n", list->tokens[current_index]->line_no, token_type_name(list->tokens[current_index]->type));
            else 
                printf("  Parsing AST, at line %d, consumed %s: %s\n", list->tokens[current_index]->line_no, token_type_name(list->tokens[current_index]->type), list->tokens[current_index]->value);
        }
        accepted_index = current_index;
        current_index++;
    }
}

// see if next token is type and accept (consume) if so.
bool accept(token_type type) {
    if (next_is(type)) {
        consume();
        return true;
    }
    return false;
}

// returns the last consumed token
token *accepted() {
    return list->tokens[accepted_index];
}

// verifies next token is of specified type, otherwise fail
bool expect(token_type type) {
    if (!next_is(type)) {
        error_at(
            list->tokens[current_index]->filename,
            list->tokens[current_index]->line_no,
            "was expecting \"%s\", but got \"%s\"", 
            token_type_name(type), 
            token_type_name(list->tokens[current_index]->type)
        );
        return false;
    }
    
    consume();
    return true;
}

// -----------------------------------
// new code
// -----------------------------------

typedef struct token_iterator_private_data {
    llist *tokens;
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

token_iterator *new_token_iterator(mempool *mp, llist *tokens) {
    token_iterator_private_data *data = mpalloc(mp, token_iterator_private_data);
    data->tokens = tokens;
    data->iter = llist_create_iterator(data->tokens, mp);
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
            token_type_name(list->tokens[current_index]->type)
        );
        return false;
    }
    
    token_iterator_consume(ti);
    return true;
}

