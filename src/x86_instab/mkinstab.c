#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


char line_buffer[1024];
char instr_section_name[32];
char *valid_modes[] = { "Valid", "N.E.", "N.S.", "Inv.", "Invalid" };

typedef struct entry {
    char *instr;
    char *op1;
    char *op2;
    char *op_en;
    char *encoding;
    bool valid64;
    bool valid32;
    char *description;
    int line_no;
} entry;

entry *entries_arr = NULL;
int entries_capacity = 0;
int entries_length = 0;


bool is_mode_identifier(char *str) {
    for (int i = 0; i < sizeof(valid_modes)/sizeof(valid_modes[0]); i++) {
        if (strcmp(str, valid_modes[i]) == 0)
            return true;
    }
    return false;
}

void add_entry_from_file(char *instr, char *op1, char *op2, char *encoding, char *op_en, 
        bool valid64, bool valid32, char *description, int line_no) {
    if (entries_capacity == 0) {
        entries_capacity = 256;
        entries_arr = malloc(entries_capacity * sizeof(entry));
    } else if (entries_length + 1 >= entries_capacity) {
        entries_capacity *= 2;
        entries_arr = realloc(entries_arr, entries_capacity * sizeof(entry));
    }
    
    entry *e = &entries_arr[entries_length];
    e->instr = strdup(instr);
    e->op1 = op1 == NULL ? NULL : strdup(op1);
    e->op2 = op2 == NULL ? NULL : strdup(op2);
    e->encoding = strdup(encoding);
    e->op_en = strdup(op_en);
    e->valid64 = valid64;
    e->valid32 = valid32;
    e->description = strdup(description);
    e->line_no = line_no;

    entries_length++;
}

void load_line(FILE *f) {
    memset(line_buffer, 0, sizeof(line_buffer));
    int len = 0;
    while (true) {
        char c = fgetc(f);
        if (feof(f) || c == '\n')
            break;
        line_buffer[len++] = c;
    }
}

void process_line(int line_no) {
    char *p = line_buffer;
    char *tokens[128];

    // empty lines and lines of comments are skipped
    while (isspace(*p)) p++;
    if (*p == '\0' || *p == '#' || strlen(p) < 2)
        return;

    // if we have a word ending in a colon, we have a instruction section
    if (p[strlen(p) - 1] == ':') {
        p[strlen(p) - 1] = 0;
        strcpy(instr_section_name, p);
        return;
    }

    memset(tokens, 0, sizeof(tokens));
    int tokens_slots = sizeof(tokens) / sizeof(tokens[0]);
    int tokens_count = 0;
    char *token = strtok(line_buffer, " \t");
    while (token != NULL && tokens_count < tokens_slots) {
        tokens[tokens_count++] = token;
        token = strtok(NULL, " \t");
    }
    if (tokens_count >= tokens_slots) {
        printf("Not enough slots in line_tokens_array, overflown at line %d\n", line_no);
        exit(1);
    }

    // find specific indexes, the instruction and the first mode column
    int instr_idx = -1;
    int mode64_idx = -1;
    for (int i = 0; i < tokens_count; i++) {
        if (instr_idx == -1 && strcmp(tokens[i], instr_section_name) == 0) {
            instr_idx = i;
        } else if (mode64_idx == -1 && is_mode_identifier(tokens[i])) {
            mode64_idx = i;
        }
    }
    if (instr_idx == -1) {
        printf("Could not find instruction '%s' in line %d\n", instr_section_name, line_no);
        exit(1);
    }
    if (mode64_idx == -1) {
        printf("Could not find mode 64 column in line %d\n", line_no);
        exit(1);
    }
    if (!is_mode_identifier(tokens[mode64_idx])) {
        printf("Invalid compatibility mode '%s' on x64 column, line %d\n", tokens[mode64_idx], line_no);
        exit(1);
    }
    if (!is_mode_identifier(tokens[mode64_idx + 1])) {
        printf("Invalid compatibility mode '%s' on compat column, line %d\n", tokens[mode64_idx + 1], line_no);
        exit(1);
    }

    // now we should be able to deduce all columns
    char encoding[128];
    strcpy(encoding, tokens[0]);
    for (int i = 1; i < instr_idx; i++) {
        strncat(encoding, " ", sizeof(encoding) - 1);
        strncat(encoding, tokens[i], sizeof(encoding) - 1);
    }

    bool has_op1 = mode64_idx - instr_idx >= 3;
    bool has_op2 = mode64_idx - instr_idx >= 4;
    bool valid64 = strcmp(tokens[mode64_idx], "Valid") == 0;
    bool valid32 = strcmp(tokens[mode64_idx + 1], "Valid") == 0;

    if (has_op1 && has_op2 && tokens[instr_idx + 1][strlen(tokens[instr_idx + 1]) - 1] == ',') {
        tokens[instr_idx + 1][strlen(tokens[instr_idx + 1]) - 1] = '\0';
    }
        
    char description[256] = {0,};
    for (int i = mode64_idx + 2; i < tokens_count; i++) {
        if (strlen(description) > 0)
            strncat(description, " ", sizeof(description) - 1);
        strncat(description, tokens[i], sizeof(description) - 1);
    }
    
    add_entry_from_file(
        tokens[instr_idx],  // instruction
        has_op1 ? tokens[instr_idx + 1] : NULL, // op1
        has_op2 ? tokens[instr_idx + 2] : NULL, // op2
        encoding,
        tokens[mode64_idx - 1], // op_en
        valid64,
        valid32,
        description,
        line_no
    );
}

bool parse_frequent_operand_type(char **ptr, char *type) {
    char *ops[]   = { "r/m",     "r",   "m",   "imm", "rel", "ptr" }; //, "moffs",    "Sreg"    };
    char *types[] = { "REG_MEM", "REG", "MEM", "IMM", "REL", "PTR" }; // , "MEM_OFFS", "SEG_REG" };
    for (int i = 0; i < sizeof(ops)/sizeof(ops[0]); i++) {
        int len = strlen(ops[i]);
        char after = (*ptr)[len]; // character after the size, e.g. "16*"
        if (memcmp(*ptr, ops[i], len) == 0 && 
            (after == 0 || isdigit(after))) {
            strcpy(type, types[i]);
            (*ptr) += len;
            return true;
        }
    }
    return false;
}

bool parse_frequent_operand_size(char **ptr, char *size) {
    char *sizes[] = { "8", "16", "32", "64" };
    for (int i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        int len = strlen(sizes[i]);
        char after = (*ptr)[len]; // character after the size, e.g. "16*"
        if (memcmp(*ptr, sizes[i], len) == 0 && 
            (after == 0 || after == '*' || after == '1' || after == '2' || after == '3')
        ) {
            sprintf(size, "S%s", sizes[i]);
            (*ptr) += len;
            return true;
        }
    }
    return false;
}

bool parse_frequent_operand_register(char **ptr, char *type, char *size) {
    char *regs[]  = { "AL", "CL", "AX", "EAX", "RAX", "ES", "CS", "SS", "DS", "FS", "GS" };
    int   sizes[] = {   8,    8,   16,    32,    64,   16,   16,   16,   16,   16,   16  };
    for (int i = 0; i < sizeof(regs)/sizeof(regs[0]); i++) {
        if (strcmp(*ptr, regs[i]) == 0) {
            sprintf(type, "%s_REG", regs[i]);
            sprintf(size, "S%d", sizes[i]);
            (*ptr) += strlen(regs[i]);
            return true;
        }
    }
    return false;
}

bool parse_single_operand(char *op, char *type) {
    strcpy(type, op == NULL ? "0" : op);
    // // e.g. "r/m64" or "r/m32" or "imm16" or "r8"
    // if (op == NULL || strlen(op) == 0) {
    //     strcpy(type, "0");
    // } else if (parse_frequent_operand_type(&op, type)) {
    //     if (!parse_frequent_operand_size(&op, size)) {
    //         // printf("%d: unrecognized size '%s'\n", e->line_no, op);
    //         return false; // skip this entry
    //     }
    // } else if (parse_frequent_operand_register(&op, type, size)) {
    //     // we are good.
    // } else {
    //     strcpy(type, op);
    //     // printf("%d: unrecognized operand '%s'\n", e->line_no, op);
    //     return false; // skip this entry
    // }
    return true;
}

void parse_entry_encoding(char *encoding, char *bytes, char *flags) {
    char *endings[]   = { "cb", "cw", "cd", "cp", "co", "ct", "ib", "iw", "id", "io" };
    char *followers[] = { "OFF_1B", "OFF_2B", "OFF_4B", "OFF_6B", "OFF_8B", "OFF_10B", "VAL_1B", "VAL_2B", "VAL_4B", "VAL_8B" };

    strcpy(bytes, encoding);
    int len = strlen(bytes);
    strcpy(flags, "0");

    for (int i = 0; i < sizeof(endings)/sizeof(endings[0]); i++) {
        if (strcmp(bytes + len - 2, endings[i]) == 0) {
            strcpy(flags, followers[i]);
            bytes[len - 2] = 0;
            break;
        }
    }

    while (strlen(bytes) > 0 && bytes[strlen(bytes) - 1] == ' ')
        bytes[strlen(bytes) - 1] = 0;
}


void generate_entry_code_for_op1_op2(FILE *f, entry *e, char *op1, char *op2) {
    // we have only single operands here, we can parse them
    char op1type[32], op2type[32];
    if (!parse_single_operand(op1, op1type)) {
        printf("%d: unknown operand \"%s\"\n", e->line_no, op1);
        return;
    }
    if (!parse_single_operand(op2, op2type)) {
        printf("%d: unknown operand \"%s\"\n", e->line_no, op2);
        return;
    }

    fprintf(f, "    { OC_%s,%*s %s,%*s %s,%*s OE_%s,%*s \"%s\",%*s}, // %s\n",
        e->instr,    (int)( 4 - strlen(e->instr)), "",
        op1type,     (int)( 6 - strlen(op1type)), "",
        op2type,     (int)( 6 - strlen(op2type)), "",
        e->op_en,    (int)( 2 - strlen(e->op_en)), "",
        e->encoding, (int)(14 - strlen(e->encoding)), "",
        e->description
    );
}

void generate_entry_code_for_op1(FILE *f, entry *e, char *op1) {
    char buffer[32];
    if (e->op2 == NULL) {
        generate_entry_code_for_op1_op2(f, e, op1, NULL);
    } else if (memcmp(e->op2, "r/m", 3) == 0 && strlen(e->op2) > 3) {
        // r/m16, r/m32, r/m64
        strcpy(buffer, "reg"); strcat(buffer, e->op2 + 3);
        generate_entry_code_for_op1_op2(f, e, op1, buffer);
        strcpy(buffer, "mem"); strcat(buffer, e->op2 + 3);
        generate_entry_code_for_op1_op2(f, e, op1, buffer);
    } else if (strchr(e->op2, '/') != NULL) {
        // multiple outputs e.g. "r16/r32"
        strcpy(buffer, e->op2);
        char *part = strtok(buffer, "/");
        while (part != NULL) {
            generate_entry_code_for_op1_op2(f, e, op1, part);
            part = strtok(NULL, "/");
        }
    } else {
        generate_entry_code_for_op1_op2(f, e, op1, e->op2);
    }
}

void generate_entry_code(FILE *f, entry *e) {
    char buffer[32];
    if (e->op1 == NULL) {
        generate_entry_code_for_op1(f, e, NULL);
    } else if (memcmp(e->op1, "r/m", 3) == 0 && strlen(e->op1) > 3) {
        // r/m16, r/m32, r/m64
        strcpy(buffer, "reg"); strcat(buffer, e->op1 + 3);
        generate_entry_code_for_op1(f, e, buffer);
        strcpy(buffer, "mem"); strcat(buffer, e->op1 + 3);
        generate_entry_code_for_op1(f, e, buffer);
    } else if (strchr(e->op1, '/') != NULL) {
        // multiple outputs e.g. "r16/r32"
        strcpy(buffer, e->op1);
        char *part = strtok(buffer, "/");
        while (part != NULL) {
            generate_entry_code_for_op1(f, e, part);
            part = strtok(NULL, "/");
        }
    } else {
        generate_entry_code_for_op1(f, e, e->op1);
    }
}

void generate_table_include_file(FILE *out) {
    fprintf(out, "// auto generated file using mkinstrtab, do not edit manually\n\n");

    fprintf(out, "struct asm_instruction_encoding_info {\n");
    fprintf(out, "    enum opcode instr;\n");
    fprintf(out, "    enum optype op1type;\n");
    fprintf(out, "    enum optype op2type;\n");
    fprintf(out, "    enum open   op_en;\n");
    fprintf(out, "    char *enc_bytes;\n");
    fprintf(out, "    enum enc_flags;\n");
    fprintf(out, "};\n\n");

    fprintf(out, "struct asm_instruction_encoding_info compat_instruction_encodings[] = {\n");
    for (int i = 0; i < entries_length; i++) {
        if (entries_arr[i].valid32)
            generate_entry_code(out, &entries_arr[i]);
    }
    fprintf(out, "};\n\n");

    fprintf(out, "struct asm_instruction_encoding_info x64_instruction_encodings[] = {\n");
    for (int i = 0; i < entries_length; i++) {
        if (entries_arr[i].valid64)
            generate_entry_code(out, &entries_arr[i]);
    }
    fprintf(out, "};\n\n");
}

int cmp_entry_by_encoding(const void *a, const void *b)    { return strcmp(((entry *)a)->encoding, ((entry *)b)->encoding); }
int cmp_entry_by_instruction(const void *a, const void *b) { return strcmp(((entry *)a)->instr,    ((entry *)b)->instr); }
int cmp_entry_by_op_en(const void *a, const void *b)       { return strcmp(((entry *)a)->op_en,    ((entry *)b)->op_en); }

// program to parse the instructions.dat file, 
// which is a copy/paste from Intel's manual,
// and generate code for our program
void main(int argc, char **argv) {
    char *in_filename  = argc > 1 ? argv[1] : "instab.dat";
    char *out_filename = argc > 2 ? argv[2] : "instab.h";

    printf("Reading '%s'...\n", in_filename);
    FILE *in = fopen(in_filename, "r");
    if (in == NULL) {
        printf("Cannot open file '%s' for reading\n", in_filename);
        exit(1);
    }

    int line_no = 1;
    load_line(in);
    while (!feof(in)) {
        process_line(line_no++);
        load_line(in);
    }

    printf("Producing '%s'...\n", out_filename);
    FILE *out = fopen(out_filename, "w");
    if (out == NULL) {
        printf("Cannot open file '%s' for reading\n", in_filename);
        exit(1);
    }
    generate_table_include_file(out);
}
