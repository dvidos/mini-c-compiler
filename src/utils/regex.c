#include "regex.h"
#include "data_types/str.h"
#include "data_structs/list.h"
#include <string.h>


#define MAX_GROUPS_SUPPORTED   9  // plus zero, the whole match
#define GROUP_NUM_STACK_SIZE   4  // nested groups depth

enum regex_token_type {
    RT_TEXT_START,
    RT_TEXT_END,
    RT_MATCH_ANYTHING,
    RT_MATCH_SPECIFIC,
    RT_WORD_BOUNDARY,
    RT_START_GROUP,
    RT_END_GROUP,
};

typedef struct regex_token regex_token;
typedef struct regex_group regex_group;

struct regex_token {
    enum regex_token_type type;
    char allowed_character;   // if only one
    const char *allowed_characters; // if many characters
    bool negated; // match anything BUT the specified character(s)
    int min_len;
    int max_len;
    int group_no;
    struct regex_token *next;
};

struct regex_group {
    bool is_open;
    str *matched_text;
};

struct regex {
    // parse / compile time data
    regex_token *token_list;
    int max_group_no;
    mempool *mempool;

    // matching time data
    regex_group groups[1 + MAX_GROUPS_SUPPORTED];
};

static char *whitespace_chars = " \t\r\n";
static char *digit_chars = "0123456789";
static char *word_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";


// ------------------ parsing / compiling a pattern ------------------


static inline bool accept(const char *string, int *pos, char acceptable) {
    if (string[*pos] != acceptable)
        return false;
    (*pos)++;
    return true;
}

static inline char accept_either(const char *string, int *pos, char acceptable1, char acceptable2) {
    char c = string[*pos];
    if (c == acceptable1 || c == acceptable2) {
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

static regex_token *parse_token_at_pattern_position(regex *re, 
        const char *pattern, int len, int *pos, 
        int grpnum_stack[], int *grpnum_stack_len) {
    regex_token *token = mpalloc(re->mempool, regex_token);
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
        if (!parse_character_ranges_at_pos(re->mempool, token, pattern, len, pos))
            return NULL;
        if (!accept(pattern, pos, ']')) {
            printf("Error in pattern, at %d, was expecting ']'", *pos);
            return NULL;
        }

    } else if (accept(pattern, pos, '\\')) {
        if (!parse_escaped_char_at_pos(token, pattern, len, pos))
            return NULL;

    } else if (accept(pattern, pos, '(')) {
        token->type = RT_START_GROUP;
        token->group_no = ++re->max_group_no;
        if ((*grpnum_stack_len) < GROUP_NUM_STACK_SIZE)
            grpnum_stack[(*grpnum_stack_len)++] = token->group_no;

    } else if (accept(pattern, pos, ')')) {
        token->type = RT_END_GROUP;
        token->group_no = ((*grpnum_stack_len) <= 0) ? 0 : grpnum_stack[--(*grpnum_stack_len)];

    } else {
        token->type = RT_MATCH_SPECIFIC;
        token->allowed_character = pattern[*pos];
        (*pos)++;
    }

    if (!parse_possible_size_at_pos(token, pattern, len, pos))
        return NULL;

    return token;
}

static regex_token *parse_pattern_into_token_list(regex *re, const char *pattern) {
    regex_token *list_head = NULL;
    regex_token *list_tail = NULL;
    int len = strlen(pattern);
    int grpnum_stack[GROUP_NUM_STACK_SIZE];
    int grpnum_stack_len = 0;
    int pos = 0;

    while (pos < len) {
        regex_token *token = parse_token_at_pattern_position(re, pattern, len, &pos, 
            grpnum_stack, &grpnum_stack_len);
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

// ------------------ matching a precompiled pattern ------------------

static bool regex_token_matches_at_pos(regex *re, regex_token *token, const char *text, int text_len, int text_pos, int *matched_len) {
    int len;
    
    switch (token->type) {
        case RT_TEXT_START:
            *matched_len = 0;
            return text_pos == 0;

        case RT_TEXT_END:
            *matched_len = 0;
            return text_pos == text_len;

        case RT_MATCH_ANYTHING:
            // greedily discover matchable length
            len = text_len - text_pos;
            if (len < token->min_len)
                return false; // not enough chars present

            if (len > token->max_len)
                len = token->max_len;
            *matched_len = len;
            return true;

        case RT_MATCH_SPECIFIC:
            // greedily discover matchable length
            len = 0;
            while (text_pos + len < text_len) {
                char c = text[text_pos + len];
                bool c_matches;
                if (token->allowed_character != 0)
                    c_matches = (c == token->allowed_character);
                else if (token->allowed_characters != NULL)
                    c_matches = (strchr(token->allowed_characters, c) != NULL);
                if (token->negated)
                    c_matches = !c_matches;
                if (!c_matches)
                    break;
                
                len++;
            }
            if (len < token->min_len)
                return false; // not enough chars present
            
            if (len > token->max_len)
                len = token->max_len;
            *matched_len = len;
            return true;

        case RT_WORD_BOUNDARY:
            // see if previous work and now non-word or vice-versa
            // or at start or end of pattern
            if (text_pos == 0 || text_pos == text_len)
                return true;
            bool prev_is_word = strchr(word_chars, text[text_pos - 1]);
            bool curr_is_word = strchr(word_chars, text[text_pos]);
            bool is_word_boundary = ((prev_is_word && !curr_is_word) || (!prev_is_word && curr_is_word));
            *matched_len = 0;
            return is_word_boundary;

        case RT_START_GROUP:
            // these always match
            re->groups[token->group_no].is_open = true;
            *matched_len = 0;
            return true;

        case RT_END_GROUP:
            // these always match
            re->groups[token->group_no].is_open = false;
            *matched_len = 0;
            return true;
    }

    printf("Unknown regex token type %d", token->type);
    return false;
}

regex *new_regex(mempool *mp, const char *pattern) {
    regex *re = mpalloc(mp, regex);
    re->mempool = mp;
    re->token_list = parse_pattern_into_token_list(re, pattern);
    for (int i = 0; i <= MAX_GROUPS_SUPPORTED; i++)
        re->groups[i].matched_text = new_str(mp, NULL);
    return re;
}

bool regex_matches(regex *re, const char *text, list *group_matches) {

    regex_token *token = re->token_list;
    int text_len = strlen(text);
    int text_pos = 0; // we s at the start of text
    int matched_len = 0;

    // reset runtime info
    for (int i = 0; i <= re->max_group_no && i <= MAX_GROUPS_SUPPORTED; i++) {
        re->groups[i].is_open = false;
        str_clear(re->groups[i].matched_text);
    }

    re->groups[0].is_open = true;
    while (token != NULL) {
        if (!regex_token_matches_at_pos(re, token, text, text_len, text_pos, &matched_len))
            return false;
        
        for (int i = 0; i <= re->max_group_no; i++)
            if (re->groups[i].is_open)
                str_catsn(re->groups[i].matched_text, text + text_pos, matched_len);
                
        text_pos += matched_len;
        token = token->next;
    }
    re->groups[0].is_open = false;

    // we finished all tokens, ensure we used all text
    if (text_pos < text_len)
        return false;

    // if placeholder provided, store the group matches
    if (group_matches != NULL) {
        list_clear(group_matches);
        for (int i = 0; i <= re->max_group_no; i++)
            list_add(group_matches, str_clone(re->groups[i].matched_text));
    }

    return true;
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

    assert(regex_matches(new_regex(mp, "."), "a", NULL));
    assert(regex_matches(new_regex(mp, ".."), "ab", NULL));

    assert( regex_matches(new_regex(mp, "[a]"), "a", NULL));
    assert(!regex_matches(new_regex(mp, "[^a]"), "a", NULL));
    assert( regex_matches(new_regex(mp, "[abc]"), "c", NULL));
    assert( regex_matches(new_regex(mp, "[a-f]"), "d", NULL));
    assert(!regex_matches(new_regex(mp, "[a-f]"), "z", NULL));
    assert( regex_matches(new_regex(mp, "[^a-f]"), "z", NULL));

    assert(regex_matches(new_regex(mp, "\\d"), "5", NULL));
    assert(!regex_matches(new_regex(mp, "\\D"), "5", NULL));
    assert(regex_matches(new_regex(mp, "\\D"), "a", NULL));

    assert( regex_matches(new_regex(mp, "\\w"), "a", NULL));
    assert(!regex_matches(new_regex(mp, "\\W"), "a", NULL));
    assert( regex_matches(new_regex(mp, "\\W"), " ", NULL));

    assert(regex_matches(new_regex(mp, "\\s"), " ", NULL));
    assert(!regex_matches(new_regex(mp, "\\S"), " ", NULL));
    assert(regex_matches(new_regex(mp, "\\S"), "a", NULL));

    assert(regex_matches(new_regex(mp, "a"), "a", NULL));
    assert(!regex_matches(new_regex(mp, "a"), "b", NULL));
    assert(regex_matches(new_regex(mp, "bb"), "bb", NULL));

    assert(regex_matches(new_regex(mp, "\\a"), "a", NULL));

    // test matching sizes

    assert( regex_matches(new_regex(mp, "a"), "a", NULL));
    assert(!regex_matches(new_regex(mp, "a"), "aa", NULL));
    assert(!regex_matches(new_regex(mp, "aa"), "a", NULL));

    assert( regex_matches(new_regex(mp, "a?"), "", NULL));
    assert( regex_matches(new_regex(mp, "a?"), "a", NULL));
    assert(!regex_matches(new_regex(mp, "a?"), "aa", NULL));

    assert(!regex_matches(new_regex(mp, "a+"), "", NULL));
    assert( regex_matches(new_regex(mp, "a+"), "a", NULL));
    assert( regex_matches(new_regex(mp, "a+"), "aa", NULL));

    assert(regex_matches(new_regex(mp, "a*"), "", NULL));
    assert(regex_matches(new_regex(mp, "a*"), "a", NULL));
    assert(regex_matches(new_regex(mp, "a*"), "aa", NULL));

    assert(!regex_matches(new_regex(mp, "a{2}"), "", NULL));
    assert(!regex_matches(new_regex(mp, "a{2}"), "a", NULL));
    assert( regex_matches(new_regex(mp, "a{2}"), "aa", NULL));
    assert(!regex_matches(new_regex(mp, "a{2}"), "aaa", NULL));

    assert(!regex_matches(new_regex(mp, "a{2,4}"), "a", NULL));
    assert( regex_matches(new_regex(mp, "a{2,4}"), "aa", NULL));
    assert( regex_matches(new_regex(mp, "a{2,4}"), "aaa", NULL));
    assert( regex_matches(new_regex(mp, "a{2,4}"), "aaaa", NULL));
    assert(!regex_matches(new_regex(mp, "a{2,4}"), "aaaaa", NULL));

    assert(!regex_matches(new_regex(mp, ".\\bcc\\b."), "acca", NULL));
    assert(!regex_matches(new_regex(mp, ".\\bcc\\b."), "acc ", NULL));
    assert(!regex_matches(new_regex(mp, ".\\bcc\\b."), " cca", NULL));
    assert( regex_matches(new_regex(mp, ".\\bcc\\b."), " cc ", NULL));

    // test groups
    list *matches = new_list(mp);
    assert(regex_matches(new_regex(mp, "((\\d+)/(\\d+))/(\\d+)"), "10/16/1975", matches));
    assert(list_length(matches) == 5);
    assert(str_cmps(list_get(matches, 0), "10/16/1975") == 0);
    assert(str_cmps(list_get(matches, 1), "10/16") == 0);
    assert(str_cmps(list_get(matches, 2), "10") == 0);
    assert(str_cmps(list_get(matches, 3), "16") == 0);
    assert(str_cmps(list_get(matches, 4), "1975") == 0);

    // test partial search

    // test substitution

    mempool_release(mp);
}
#endif


