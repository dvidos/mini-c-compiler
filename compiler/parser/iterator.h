#pragma once

#include "stddef.h"
#include "stdio.h"
#include "stdarg.h"
#include "../token.h"

void init_token_iterator(token *first_token);

// checks the type of the next token, without advancing
bool next_is(token_type type);

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
void expect(token_type type);

