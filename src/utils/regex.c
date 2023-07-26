#include "regex.h"
#include <string.h>



enum regex_token_type {
    RT_TEXT_START,
    RT_TEXT_END,
    RT_MATCH_ANYTHING,
    RT_MATCH_SPECIFIC,
    RT_WORD_BOUNDARY,
    RT_START_GROUP,
    RT_END_GROUP,
};

typedef struct regex_token {
    enum regex_token_type type;
    char allowed_character;   // if only one
    const char *allowed_characters; // if many characters
    bool negated; // match anything BUT the specified character(s)
    int min_len;
    int max_len;
    struct regex_token *next;
} regex_token;

typedef struct regex {
    regex_token *token_list;
    // also matching groups list would be here

    mempool *mempool;
} regex;

static char *whitespace_chars = " \t\r\n";
static char *digit_chars = "0123456789";
static char *word_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";

static inline bool accept(const char *string, int *pos, char acceptable) {
    if (string[*pos] != acceptable)
        return false;
    (*pos)++;
    return true;
}

static inline char accept_either(const char *string, int *pos, char accept1, char accept2) {
    char c = string[*pos];
    if (c == accept1 || c == accept2) {
        (*pos)++;
        return c;
    }
    return 0;
}

static bool parse_escaped_char_at_pos(regex_token *token, const char *pattern, int len, int *pos) {
    // what can be following an escape char?
    char c;
    if ((c = accept_either(pattern, pos, 'd', 'D')) != 0) {
        token->type = RT_MATCH_SPECIFIC;
        token->allowed_characters = digit_chars;
        token->negated = (c == 'D');
    } else if ((c = accept_either(pattern, pos, 'w', 'W')) != 0) {
        token->type = RT_MATCH_SPECIFIC;
        token->allowed_characters = word_chars;
        token->negated = (c == 'W');
    } else if ((c = accept_either(pattern, pos, 's', 'S')) != 0) {
        token->type = RT_MATCH_SPECIFIC;
        token->allowed_characters = whitespace_chars;
        token->negated = (c == 'S');
    } else if (accept(pattern, pos, 'b')) {
        token->type = RT_WORD_BOUNDARY;
    } else {
        token->type = RT_MATCH_SPECIFIC;
        token->allowed_character = pattern[*pos];
        (*pos)++;
    }
    return true;
}

static bool parse_number_at_pos(const char *pattern, int len, int *pos, int *number) {
    // things inside "{}"
    *number = 0;
    char c = pattern[*pos];
    while (c >= '0' && c <= '9') {
        (*number) = (*number) * 10 + (c - '0');
        (*pos)++;
        c = pattern[*pos];
    }
    return true;
}

static bool parse_character_ranges_at_pos(mempool *mp, regex_token *token, const char *pattern, int len, int *pos) {
    str *tmp = new_str(mp, NULL);

    // things inside "[]"
    char chr = pattern[*pos];
    while (chr != ']') {
        str_catc(tmp, chr);
        (*pos)++; // skip initial char

        if (pattern[*pos] == '-') {
            (*pos)++; // skip dash
            for (char rng = (chr + 1); rng <= pattern[(*pos)]; rng++)
                str_catc(tmp, rng);
            (*pos)++; // skip final char
        }

        if ((*pos) >= len) {
            printf("Expected closing bracket not found");
            return false;
        }
        chr = pattern[*pos];
    }
    token->allowed_characters = str_charptr(tmp);
    return true;
}

static bool parse_possible_size_at_pos(regex_token *token, const char *pattern, int len, int *pos) {
    int number;

    if (accept(pattern, pos, '?')) {
        token->min_len = 0;
        token->max_len = 1;
    } else if (accept(pattern, pos, '*')) {
        token->min_len = 0;
        token->max_len = INT32_MAX;
    } else if (accept(pattern, pos, '+')) {
        token->min_len = 1;
        token->max_len = INT32_MAX;
    } else if (accept(pattern, pos, '{')) {
        if (!parse_number_at_pos(pattern, len, pos, &number))
            return false;
        token->min_len = number;
        token->max_len = number;
        if (accept(pattern, pos, ',')) {
            if (!parse_number_at_pos(pattern, len, pos, &number))
                return false;
            token->max_len = number;
        }
        if (!accept(pattern, pos, '}')) {
            printf("Error in pattern, at %d, was expecting '}'", *pos);
            return NULL;
        }
    }
    return true;
}

static regex_token *parse_token_at_pattern_position(mempool *mp, const char *pattern, int len, int *pos) {
    regex_token *token = mpalloc(mp, regex_token);
    memset(token, 0, sizeof(regex_token)); // explicit reset

    // if no explicit size is defined, 1..1 is assumed
    token->min_len = 1;
    token->max_len = 1;
    
    if (accept(pattern, pos, '.')) {
        token->type = RT_MATCH_ANYTHING;

    } else if (accept(pattern, pos, '^')) {
        token->type = RT_TEXT_START;

    } else if (accept(pattern, pos, '$')) {
        token->type = RT_TEXT_END;

    } else if (accept(pattern, pos, '[')) {
        token->type = RT_MATCH_SPECIFIC;
        if (accept(pattern, pos, '^'))
            token->negated = true;
        if (!parse_character_ranges_at_pos(mp, token, pattern, len, pos))
            return NULL;
        if (!accept(pattern, pos, ']')) {
            printf("Error in pattern, at %d, was expecting ']'", *pos);
            return NULL;
        }

    } else if (accept(pattern, pos, '\\')) {
        if (!parse_escaped_char_at_pos(token, pattern, len, pos))
            return NULL;

    } else {
        token->type = RT_MATCH_SPECIFIC;
        token->allowed_character = pattern[*pos];
        (*pos)++;
    }

    if (!parse_possible_size_at_pos(token, pattern, len, pos))
        return NULL;

    return token;
}

static regex_token *parse_pattern_into_token_list(mempool *mp, const char *pattern) {
    regex_token *list_head = NULL;
    regex_token *list_tail = NULL;
    int len = strlen(pattern);
    int pos = 0;

    while (pos < len) {
        regex_token *token = parse_token_at_pattern_position(mp, pattern, len, &pos);
        if (token == NULL)
            break;
        
        if (list_head == NULL) {
            list_head = token;
            list_tail = token;
        } else {
            list_tail->next = token;
            list_tail = token;
        }
    }

    return list_head;
}


static bool regex_token_matches_at_pos(regex_token *token, const char *text, int len, int *pos) {
    int matchable_len;

    switch (token->type) {
        case RT_TEXT_START:
            return (*pos == 0);

        case RT_TEXT_END:
            return (*pos == len);

        case RT_MATCH_ANYTHING:
            // greedily discover matchable length
            matchable_len = len - (*pos);
            if (matchable_len < token->min_len)
                return false; // not enough chars present

            if (matchable_len > token->max_len)
                matchable_len = token->max_len;
            (*pos) += matchable_len;
            return true;

        case RT_MATCH_SPECIFIC:
            // greedily discover matchable length
            matchable_len = 0;
            while ((*pos) + matchable_len < len) {
                char c = text[(*pos) + matchable_len];
                bool c_matches;
                if (token->allowed_character != 0)
                    c_matches = (c == token->allowed_character);
                else if (token->allowed_characters != NULL)
                    c_matches = (strchr(token->allowed_characters, c) != NULL);
                if (token->negated)
                    c_matches = !c_matches;
                if (!c_matches)
                    break;
                
                matchable_len++;
            }
            if (matchable_len < token->min_len)
                return false; // not enough chars present
            
            if (matchable_len > token->max_len)
                matchable_len = token->max_len;
            (*pos) += matchable_len;
            return true;

        case RT_WORD_BOUNDARY:
            // see if previous work and now non-word or vice-versa
            // or at start or end of pattern
            if ((*pos) == 0 || (*pos) == len - 1)
                return true;
            bool prev_is_word = strchr(word_chars, text[(*pos) - 1]);
            bool curr_is_word = strchr(word_chars, text[(*pos)]);
            return ((prev_is_word && !curr_is_word) || (!prev_is_word && curr_is_word));

        case RT_START_GROUP:
            // these always match
            return true;

        case RT_END_GROUP:
            // these always match
            return true;
    }

    printf("Unknown regex token type %d", token->type);
    return false;
}

regex *new_regex(mempool *mp, const char *pattern) {
    regex *re = mpalloc(mp, regex);
    re->token_list = parse_pattern_into_token_list(mp, pattern);
    re->mempool = mp;
    return re;
}

bool regex_matches(regex *re, const char *text) {
    regex_token *token = re->token_list;
    int len = strlen(text);
    int pos = 0; // we start at the start of text
    while (token != NULL) {
        if (!regex_token_matches_at_pos(token, text, len, &pos))
            return false;
        token = token->next;
    }

    // we finished all tokens, ensure we used all text
    return (pos == len);
}

bool regex_search(regex *re, const char *text, list *groups) {
    return false;
}

str *regex_replace(regex *re, const char *text, const char *replacement) {
    return NULL;
}

#ifdef INCLUDE_UNIT_TESTS
void regex_unit_tests() {
    // test all!
    mempool *mp = new_mempool();

    // test matching content

    assert(regex_matches(new_regex(mp, "."), "a"));
    assert(regex_matches(new_regex(mp, ".."), "ab"));

    assert( regex_matches(new_regex(mp, "[a]"), "a"));
    assert(!regex_matches(new_regex(mp, "[^a]"), "a"));
    assert( regex_matches(new_regex(mp, "[abc]"), "c"));
    assert( regex_matches(new_regex(mp, "[a-f]"), "d"));
    assert(!regex_matches(new_regex(mp, "[a-f]"), "z"));
    assert( regex_matches(new_regex(mp, "[^a-f]"), "z"));

    assert(regex_matches(new_regex(mp, "\\d"), "5"));
    assert(!regex_matches(new_regex(mp, "\\D"), "5"));
    assert(regex_matches(new_regex(mp, "\\D"), "a"));

    assert( regex_matches(new_regex(mp, "\\w"), "a"));
    assert(!regex_matches(new_regex(mp, "\\W"), "a"));
    assert( regex_matches(new_regex(mp, "\\W"), " "));

    assert(regex_matches(new_regex(mp, "\\s"), " "));
    assert(!regex_matches(new_regex(mp, "\\S"), " "));
    assert(regex_matches(new_regex(mp, "\\S"), "a"));

    assert(regex_matches(new_regex(mp, "a"), "a"));
    assert(!regex_matches(new_regex(mp, "a"), "b"));
    assert(regex_matches(new_regex(mp, "bb"), "bb"));

    assert(regex_matches(new_regex(mp, "\\a"), "a"));

    // test matching sizes

    assert( regex_matches(new_regex(mp, "a"), "a"));
    assert(!regex_matches(new_regex(mp, "a"), "aa"));
    assert(!regex_matches(new_regex(mp, "aa"), "a"));

    assert( regex_matches(new_regex(mp, "a?"), ""));
    assert( regex_matches(new_regex(mp, "a?"), "a"));
    assert(!regex_matches(new_regex(mp, "a?"), "aa"));

    assert(!regex_matches(new_regex(mp, "a+"), ""));
    assert( regex_matches(new_regex(mp, "a+"), "a"));
    assert( regex_matches(new_regex(mp, "a+"), "aa"));

    assert(regex_matches(new_regex(mp, "a*"), ""));
    assert(regex_matches(new_regex(mp, "a*"), "a"));
    assert(regex_matches(new_regex(mp, "a*"), "aa"));

    assert(!regex_matches(new_regex(mp, "a{2}"), ""));
    assert(!regex_matches(new_regex(mp, "a{2}"), "a"));
    assert( regex_matches(new_regex(mp, "a{2}"), "aa"));
    assert(!regex_matches(new_regex(mp, "a{2}"), "aaa"));

    assert(!regex_matches(new_regex(mp, "a{2,4}"), "a"));
    assert( regex_matches(new_regex(mp, "a{2,4}"), "aa"));
    assert( regex_matches(new_regex(mp, "a{2,4}"), "aaa"));
    assert( regex_matches(new_regex(mp, "a{2,4}"), "aaaa"));
    assert(!regex_matches(new_regex(mp, "a{2,4}"), "aaaaa"));

    // test SOT, EOT, word boundaries

    // test partial search

    // test groups

    // test substitution

    mempool_release(mp);
}
#endif


