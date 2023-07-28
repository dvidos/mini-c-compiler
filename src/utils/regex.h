#pragma once
#include "unit_tests.h"
#include "data_types/str.h"
#include "mempool.h"

/*  things supported:
    ^ sot, $ eot, \b word boundary
    content
        .  anything
        [] inclusion, [^] exclusion
        \w word, \d digit, \s space
        all else matches itself, length 1
    length
        ? zero or one
        * zero or more
        + one or more
        {num}, {min,max} specific len(s)
    special
        ()  define a group
        \\  escape next char (e.g. "\[")
*/

typedef struct regex regex;

regex *new_regex(mempool *mp, const char *pattern);

// match the whole string
bool regex_matches(regex *re, const char *text, list *group_matches);

// find pattern in any place in text, fill in the groups list
bool regex_search(regex *re, const char *text, list *groups);

// find pattern in any place in text, return replacement. \n or \{n} for groups
str *regex_replace(regex *re, const char *text, const char *replacement);


#ifdef INCLUDE_UNIT_TESTS
void regex_unit_tests();
#endif
