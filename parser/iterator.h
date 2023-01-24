#pragma once

#include "stddef.h"
#include "stdio.h"
#include "stdarg.h"
#include "../lexer/token.h"

void init_token_iterator(token *first_token);

// returns the next (aka current) token, as opposed to accepted()
token *next();

// allows examination of subsequent tokens
token *lookahead(int times);

// checks the type of the next token, without advancing
bool next_is(token_type type);

// checks the type of subsequent tokens, without advancing
bool lookahead_is(int times, token_type type);

// advances to the next token
void consume();

// show a message and flag parsing failure
void parsing_error(char *msg, ...);

// if an error occured during parsing
bool parsing_failed();

// see if next token is type and accept (consume) if so.
bool accept(token_type type);

// returns the last consumed token
token *accepted();

// verifies next token is of specified type, otherwise fail
bool expect(token_type type);

