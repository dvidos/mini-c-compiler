#include <string.h>
#include "../err_handler.h"


#define is_eof(c)         ((c)=='\0')
#define is_whitespace(c)  ((c)==' ' || (c)=='\t')
#define is_newline(c)     ((c)=='\n' || (c)=='\r')
#define is_label(c)       ((c)==':')
#define is_comma(c)       ((c)==',')
#define is_comment(c)     ((c)==';')
#define is_alphanum(c)    (((c)>='a'&&(c)<='z') || ((c)>='A'&&(c)<='Z') || ((c)>='0'&&(c)<='9') || (c)=='_')
#define is_op_symbol(c)   (((c)=='[') || ((c)==']') || ((c)=='+') || ((c)=='-'))


static inline void skip_whitespace(char **stream) {
    while (is_whitespace(**stream)) (*stream)++;
}

static inline void skip_to_next_line(char **stream) {
    while (!is_newline(**stream) && !is_eof(**stream)) (*stream)++;
    while (is_newline(**stream)) (*stream)++;
}

static inline void skip_to_eof(char **stream) {
    while (!is_eof(**stream)) (*stream)++;
}

static inline void grab_token(char **stream, char *target, int target_size) {
    target_size--; // to allow for terminator
    while (is_alphanum(**stream) || is_op_symbol(**stream)) {
        *target++ = **stream;
        (*stream)++;
        if (--target_size == 0)
            break;
    }
    *target = '\0';
}



// expecting the following format:
//       <label>:   <opcode> [ <op1> [ , <op2> ]] [ ; line comment ]
// label is expected on first column, opcode must be after whitespace
// returns true if we gotten the opcode, all the rest are optional.
void parse_asm_line(char **stream, char *label, char *opcode, char *op1, char *op2, int line_no, int str_size) {
    char *placeholders[4];

    placeholders[0] = label;
    placeholders[1] = opcode;
    placeholders[2] = op1;
    placeholders[3] = op2;

    // parts: label, opcode, op1, op2
    // assume we are at start of the word each time
    for (int part = 0; part < 4 && !is_eof(**stream); part++) {
        *placeholders[part] = '\0';
        grab_token(stream, placeholders[part], 64);
        if (part == 0 && strlen(placeholders[part]) > 0 && !is_label(**stream)) {
            error("was expecting a color after label at start of line");
            return;
        }

        skip_whitespace(stream);
        if (is_eof(**stream)) break;

        if (part == 2 && is_comma(**stream)) {
            (*stream)++;
        }

        skip_whitespace(stream);
        if (is_eof(**stream)) break;

        if (is_comment(**stream)) {
            skip_to_next_line(stream);
            break;
        }
    }
}

