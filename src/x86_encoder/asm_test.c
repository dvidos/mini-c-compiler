#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "instruction.h"
#include "encoder.h"
#include "bin_buffer.h"
#include "symbol_table.h"
#include "../elf/elf.h"
#include "../utils.h"
#include "../options.h"


static void test_create_executable();
static void test_push_pops();
static void test_movs();
static void test_instructions();
static bool verify_single_instruction(struct instruction *instr, char *expected_bytes, int expected_len);
static void verify_listing(char *title, struct instruction *list, int instr_count, char *expected, int expected_len);


#define DECLARE_LIST() \
    struct instruction list[32];    \
    int list_len = 0;             \
    memset(&list, 0, sizeof(list));

#define ADD0(op)  \
    list[list_len].opcode = op; \
    list_len++;

#define ADD1_(op, op1type, op1value) \
    list[list_len].opcode = op; \
    list[list_len].op1.type = op1type; \
    list[list_len].op1.value = op1value; \
    list_len++;

#define ADD1o(op, op1type, op1value, op1offset) \
    list[list_len].opcode = op; \
    list[list_len].op1.type = op1type; \
    list[list_len].op1.value = op1value; \
    list[list_len].op1.offset = op1offset; \
    list_len++;

#define ADD1s(op, name) \
    list[list_len].opcode = op; \
    list[list_len].op1.type = OT_SYMBOL_MEM_ADDRESS; \
    list[list_len].op1.symbol_name = name; \
    list_len++;

#define ADD2(op, op1type, op1value, op2type, op2value) \
    list[list_len].opcode = op; \
    list[list_len].op1.type = op1type; \
    list[list_len].op1.value = op1value; \
    list[list_len].op2.type = op2type; \
    list[list_len].op2.value = op2value; \
    list_len++;

#define ADD2s(op, op1type, op1value, name) \
    list[list_len].opcode = op; \
    list[list_len].op1.type = op1type; \
    list[list_len].op1.value = op1value; \
    list[list_len].op2.type = OT_SYMBOL_MEM_ADDRESS; \
    list[list_len].op2.symbol_name = name; \
    list_len++;

#define ADDs2(op, name, op2type, op2value) \
    list[list_len].opcode = op; \
    list[list_len].op1.type = OT_SYMBOL_MEM_ADDRESS; \
    list[list_len].op1.symbol_name = name; \
    list[list_len].op2.type = op2type; \
    list[list_len].op2.value = op2value; \
    list_len++;


void perform_asm_test() {
    // test_push_pops();
    // test_movs();
    //test_instructions();
    test_create_executable();
}


static void test_push_pops() {
    struct instruction list[16];
    int list_len = 0;
    memset(&list, 0, sizeof(list));

    ADD1_(OC_PUSH, OT_REGISTER, REG_AX);
    ADD1o(OC_PUSH, OT_MEM_DWORD_POINTED_BY_REG, REG_BX, +3);
    ADD1_(OC_PUSH, OT_IMMEDIATE, 0x1234);
    ADD1s(OC_PUSH, "nicholas");
    ADD1_(OC_POP, OT_REGISTER, REG_CX);
    ADD1o(OC_POP, OT_MEM_DWORD_POINTED_BY_REG, REG_DX, +2);
    ADD1s(OC_POP, "nicholas");

    // using https://defuse.ca/online-x86-assembler.htm
    // 0:  50                      push   eax
    // 1:  ff 73 03                push   DWORD PTR [ebx+0x3]
    // 4:  68 34 12 00 00          push   0x1234
    // 9:  ff 35 00 00 00 00       push   DWORD PTR ds:0x0
    // f:  59                      pop    ecx
    // 10: 8f 42 02                pop    DWORD PTR [edx+0x2]
    // 13: 8f 05 00 00 00 00       pop    DWORD PTR ds:0x0

    u8 *expected = "\x50\xFF\x73\x03\x68\x34\x12\x00\x00\xFF\x35\x00\x00\x00\x00\x59\x8F\x42\x02\x8F\x05\x00\x00\x00\x00";
    int expected_len = 25;
    verify_listing("Push operations", list, list_len, expected, expected_len);
}

static void test_movs() {
    DECLARE_LIST();

    // using https://defuse.ca/online-x86-assembler.htm
    // MOV EAX, 1
    // MOV ECX, 1
    // MOV EDX, 1
    // MOV EAX, ESP
    // MOV EAX, EBP
    // MOV EAX, ESI
    // 0:  b8 01 00 00 00          mov    eax,0x1
    // 5:  b9 01 00 00 00          mov    ecx,0x1
    // a:  ba 01 00 00 00          mov    edx,0x1
    // f:  89 e0                   mov    eax,esp  (e0 = 11 100 000)
    // 11: 89 e8                   mov    eax,ebp  (e8 = 11 101 000)
    // 13: 89 f0                   mov    eax,esi  (f0 = 11 110 000)
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_IMMEDIATE, 1);
    ADD2(OC_MOV, OT_REGISTER, REG_CX, OT_IMMEDIATE, 1);
    ADD2(OC_MOV, OT_REGISTER, REG_DX, OT_IMMEDIATE, 1);
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_REGISTER, REG_SP);
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_REGISTER, REG_BP);
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_REGISTER, REG_SI);
    ADD0(OC_NOP);

    // MOV EAX, symbol
    // MOV symbol, EAX
    // 15: a1 00 00 00 00          mov    eax,ds:0x0
    // 1a: a3 00 00 00 00          mov    ds:0x0,eax
    ADD2s(OC_MOV, OT_REGISTER, REG_AX, "nicholas");
    ADDs2(OC_MOV, "nicholas", OT_REGISTER, REG_AX);

    // MOV EAX, [ECX]
    // MOV EAX, [EBP+4]
    // MOV EAX, [EBP-2]
    // 1f: 8b 01                   mov    eax,DWORD PTR [ecx]
    // 21: 8b 45 04                mov    eax,DWORD PTR [ebp+0x4]
    // 24: 8b 45 fe                mov    eax,DWORD PTR [ebp-0x2]
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_MEM_DWORD_POINTED_BY_REG, REG_CX);
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_MEM_DWORD_POINTED_BY_REG, REG_BP);
    list[list_len-1].op2.offset = +4;
    ADD2(OC_MOV, OT_REGISTER, REG_AX, OT_MEM_DWORD_POINTED_BY_REG, REG_BP);
    list[list_len-1].op2.offset = -2;

    // MOV DWORD PTR [ECX], EAX
    // MOV DWORD PTR [ECX], 1
    // MOV DWORD PTR [EBP+4], 1
    // MOV DWORD PTR [EBP-2], 1
    // 27: 89 01                   mov    DWORD PTR [ecx],eax
    // 29: c7 01 01 00 00 00       mov    DWORD PTR [ecx],0x1
    // 2f: c7 45 04 01 00 00 00    mov    DWORD PTR [ebp+0x4],0x1
    // 36: c7 45 fe 01 00 00 00    mov    DWORD PTR [ebp-0x2],0x1
    ADD2(OC_MOV, OT_MEM_DWORD_POINTED_BY_REG, REG_CX, OT_REGISTER, REG_AX);
    ADD2(OC_MOV, OT_MEM_DWORD_POINTED_BY_REG, REG_CX, OT_IMMEDIATE, 1);
    ADD2(OC_MOV, OT_MEM_DWORD_POINTED_BY_REG, REG_BP, OT_IMMEDIATE, 1);
    list[list_len-1].op1.offset = +4;
    ADD2(OC_MOV, OT_MEM_DWORD_POINTED_BY_REG, REG_BP, OT_IMMEDIATE, 1);
    list[list_len-1].op2.offset = -2;



    char *expected = "\xB8\x01\x00\x00\x00\xB9\x01\x00\x00\x00\xBA\x01\x00\x00\x00\x89\xE0\x89\xE8\x89\xF0\xA1\x00\x00\x00\x00\xA3\x00\x00\x00\x00\x8B\x01\x8B\x45\x04\x8B\x45\xFE\x89\x01\xC7\x01\x01\x00\x00\x00\xC7\x45\x04\x01\x00\x00\x00\xC7\x45\xFE\x01\x00\x00\x00";
    int expected_len = 244 / 4;

    verify_listing("MOVs", list, list_len, expected, expected_len);
}

#define VERIFY_INSTR0(oc, expect_bytes, expect_len)   \
    instr.opcode = oc; \
    instr.op1.type = OT_NONE; \
    instr.op2.type = OT_NONE; \
    if (!verify_single_instruction(&instr, expect_bytes, expect_len)) return;

#define VERIFY_INSTR1_IMMEDIATE(code, val, expect_bytes, expect_len) \
    instr.opcode = code; \
    instr.op1.type = OT_IMMEDIATE; \
    instr.op1.value = val; \
    instr.op2.type = OT_NONE; \
    if (!verify_single_instruction(&instr, expect_bytes, expect_len)) return;

#define VERIFY_INSTR1_REGISTER(code, reg_no, expect_bytes, expect_len) \
    instr.opcode = code; \
    instr.op1.type = OT_REGISTER; \
    instr.op1.value = reg_no; \
    instr.op2.type = OT_NONE; \
    if (!verify_single_instruction(&instr, expect_bytes, expect_len)) return;

#define VERIFY_INSTR1_MEMBYREG(code, reg_no, offs, expect_bytes, expect_len) \
    instr.opcode = code; \
    instr.op1.type = OT_MEM_DWORD_POINTED_BY_REG; \
    instr.op1.value = reg_no; \
    instr.op1.offset = offs; \
    instr.op2.type = OT_NONE; \
    if (!verify_single_instruction(&instr, expect_bytes, expect_len)) return;

#define VERIFY_INSTR1_MEMBYSYM(code, sym, expect_bytes, expect_len) \
    instr.opcode = code; \
    instr.op1.type = OT_SYMBOL_MEM_ADDRESS; \
    instr.op1.symbol_name = sym; \
    instr.op2.type = OT_NONE; \
    if (!verify_single_instruction(&instr, expect_bytes, expect_len)) return;

#define VERIFY_INSTR2_REG_REG(code, target_regno, source_regno, expect_bytes, expect_len) \
    instr.opcode = code; \
    instr.op1.type = OT_REGISTER; \
    instr.op1.value = target_regno; \
    instr.op2.type = OT_REGISTER; \
    instr.op2.value = source_regno; \
    if (!verify_single_instruction(&instr, expect_bytes, expect_len)) return;

#define VERIFY_INSTR2_REG_IMMEDIATE(code, regno, val, expect_bytes, expect_len) \
    instr.opcode = code; \
    instr.op1.type = OT_REGISTER; \
    instr.op1.value = regno; \
    instr.op2.type = OT_IMMEDIATE; \
    instr.op2.value = val; \
    if (!verify_single_instruction(&instr, expect_bytes, expect_len)) return;


static void test_instructions() {
    struct instruction instr;
    printf("Verifying instructions ");

    // no operands instruction
    VERIFY_INSTR0(OC_NOP, "\x90", 1);
    VERIFY_INSTR0(OC_RET, "\xC3", 1);

    // one operand instructions: immediate, register, mem<-reg, mem<-symbol
    VERIFY_INSTR1_IMMEDIATE(OC_INT,        0x21, "\xCD\x21", 2);
    VERIFY_INSTR1_IMMEDIATE(OC_INT,        0x80, "\xCD\x80", 2);
    VERIFY_INSTR1_IMMEDIATE(OC_PUSH,          0, "\x6A\x00", 2);
    VERIFY_INSTR1_IMMEDIATE(OC_PUSH,          1, "\x6A\x01", 2);
    VERIFY_INSTR1_IMMEDIATE(OC_PUSH,         -1, "\x6A\xFF", 2);
    VERIFY_INSTR1_IMMEDIATE(OC_PUSH, 0x12345678, "\x68\x78\x56\x34\x12", 5);

    // one operand, with a register
    VERIFY_INSTR1_REGISTER(OC_PUSH, REG_AX, "\x50", 1);
    VERIFY_INSTR1_REGISTER(OC_PUSH, REG_DX, "\x52", 1);
    VERIFY_INSTR1_REGISTER(OC_POP,  REG_BX, "\x5b", 1);
    VERIFY_INSTR1_REGISTER(OC_POP,  REG_CX, "\x59", 1);
    VERIFY_INSTR1_REGISTER(OC_INC,  REG_SI, "\x46", 1);
    VERIFY_INSTR1_REGISTER(OC_INC,  REG_DI, "\x47", 1);
    VERIFY_INSTR1_REGISTER(OC_DEC,  REG_SI, "\x4e", 1);
    VERIFY_INSTR1_REGISTER(OC_DEC,  REG_DI, "\x4f", 1);
    VERIFY_INSTR1_REGISTER(OC_NOT,  REG_DX, "\xf7\xd2", 2);
    VERIFY_INSTR1_REGISTER(OC_NEG,  REG_DX, "\xf7\xda", 2);
    VERIFY_INSTR1_REGISTER(OC_CALL, REG_AX, "\xff\xd0", 2);

    // modify dword pointed by register
    VERIFY_INSTR1_MEMBYREG(OC_PUSH, REG_AX,      0, "\xff\x30",     2);
    VERIFY_INSTR1_MEMBYREG(OC_PUSH, REG_CX,      0, "\xff\x31",     2);
    VERIFY_INSTR1_MEMBYREG(OC_PUSH, REG_CX,     -4, "\xff\x71\xfc", 3);
    VERIFY_INSTR1_MEMBYREG(OC_PUSH, REG_CX,     +8, "\xff\x71\x08", 3);
    VERIFY_INSTR1_MEMBYREG(OC_POP,  REG_DX,      0, "\x8f\x02",     2);
    VERIFY_INSTR1_MEMBYREG(OC_POP,  REG_BX,      0, "\x8f\x03",     2);
    VERIFY_INSTR1_MEMBYREG(OC_INC,  REG_AX,      0, "\xff\x00",     2);
    VERIFY_INSTR1_MEMBYREG(OC_DEC,  REG_CX,      0, "\xff\x09",     2);
    VERIFY_INSTR1_MEMBYREG(OC_DEC,  REG_CX,     -4, "\xff\x49\xfc", 3);
    VERIFY_INSTR1_MEMBYREG(OC_DEC,  REG_CX,     +8, "\xff\x49\x08", 3);
    VERIFY_INSTR1_MEMBYREG(OC_DEC,  REG_CX, +0x200, "\xff\x89\x00\x02\x00\x00", 6);
    VERIFY_INSTR1_MEMBYREG(OC_NOT,  REG_DX,      0, "\xf7\x12",     2);
    VERIFY_INSTR1_MEMBYREG(OC_NEG,  REG_BX,      0, "\xf7\x1b",     2);
    VERIFY_INSTR1_MEMBYREG(OC_CALL, REG_DX,      0, "\xff\x12",     2);

    // modify dword pointed by symbol
    VERIFY_INSTR1_MEMBYSYM(OC_PUSH, "var1", "\xff\x35\x00\x00\x00\x00", 6);
    VERIFY_INSTR1_MEMBYSYM(OC_POP,  "var1", "\x8f\x05\x00\x00\x00\x00", 6);
    VERIFY_INSTR1_MEMBYSYM(OC_INC,  "var1", "\xff\x05\x00\x00\x00\x00", 6);
    VERIFY_INSTR1_MEMBYSYM(OC_DEC,  "var1", "\xff\x0d\x00\x00\x00\x00", 6);
    VERIFY_INSTR1_MEMBYSYM(OC_NOT,  "var1", "\xf7\x15\x00\x00\x00\x00", 6);
    VERIFY_INSTR1_MEMBYSYM(OC_NEG,  "var1", "\xf7\x1d\x00\x00\x00\x00", 6);
    VERIFY_INSTR1_MEMBYSYM(OC_CALL, "var1", "\x2e\xff\x15\x00\x00\x00\x00", 7);

    // // two operands operations, source & target is a register
    VERIFY_INSTR2_REG_REG(OC_MOV, REG_AX, REG_SP, "\x89\xE0", 2);
    VERIFY_INSTR2_REG_REG(OC_MOV, REG_AX, REG_BP, "\x89\xE8", 2);
    VERIFY_INSTR2_REG_REG(OC_MOV, REG_AX, REG_SI, "\x89\xF0", 2);
    VERIFY_INSTR2_REG_REG(OC_MOV, REG_AX, REG_DI, "\x89\xF8", 2);
    VERIFY_INSTR2_REG_REG(OC_MOV, REG_CX, REG_DI, "\x89\xF9", 2);
    VERIFY_INSTR2_REG_REG(OC_MOV, REG_DX, REG_DI, "\x89\xFA", 2);
    VERIFY_INSTR2_REG_REG(OC_ADD, REG_AX, REG_DX, "\x01\xD0", 2);
    VERIFY_INSTR2_REG_REG(OC_SUB, REG_AX, REG_DX, "\x29\xD0", 2);
    VERIFY_INSTR2_REG_REG(OC_AND, REG_AX, REG_DX, "\x21\xD0", 2);
    VERIFY_INSTR2_REG_REG(OC_OR,  REG_AX, REG_DX, "\x09\xD0", 2);
    VERIFY_INSTR2_REG_REG(OC_XOR, REG_AX, REG_DX, "\x31\xD0", 2);

    VERIFY_INSTR2_REG_IMMEDIATE(OC_MOV, REG_DX, 0x0,        "\xBA\x00\x00\x00\x00", 5);
    VERIFY_INSTR2_REG_IMMEDIATE(OC_MOV, REG_DX, 0x1,        "\xBA\x01\x00\x00\x00", 5);
    VERIFY_INSTR2_REG_IMMEDIATE(OC_MOV, REG_DX, 0x12345678, "\xBA\x78\x56\x34\x12", 5);
    VERIFY_INSTR2_REG_IMMEDIATE(OC_ADD, REG_DX, 0x200,      "\x81\xC2\x00\x02\x00\x00", 6);
    VERIFY_INSTR2_REG_IMMEDIATE(OC_SUB, REG_DX, 0x200,      "\x81\xEA\x00\x02\x00\x00", 6);
    VERIFY_INSTR2_REG_IMMEDIATE(OC_SHR, REG_DX, 0x3,        "\xC1\xEA\x03", 3);
    VERIFY_INSTR2_REG_IMMEDIATE(OC_SHL, REG_DX, 0x6,        "\xC1\xE2\x06", 3);
    VERIFY_INSTR2_REG_IMMEDIATE(OC_AND, REG_DX, 0xFF00,     "\x81\xE2\x00\xFF\x00\x00", 6);
    VERIFY_INSTR2_REG_IMMEDIATE(OC_OR,  REG_DX, 0xFF00,     "\x81\xCA\x00\xFF\x00\x00", 6);
    VERIFY_INSTR2_REG_IMMEDIATE(OC_XOR, REG_DX, 0x5555,     "\x81\xF2\x55\x55\x00\x00", 6);
    VERIFY_INSTR2_REG_IMMEDIATE(OC_CMP, REG_DX, 0x200,      "\x81\xFA\x00\x02\x00\x00", 6);

    // VERIFY_INSTR2_REG_MEMBYREG(opcode, target_regno, mem_regno, mem_offset, expect_bytes, expect_len);
    // VERIFY_INSTR2_REG_MEMBYSYM(opcode, target_regno, symbol_name, expect_bytes, expect_len);

    // // two operands operations, target is an address pointed by register +/- offset
    // VERIFY_INSTR2_MEMBYREG_REGISTER(opcode, mem_regno, mem_offset, source_regno, expect_bytes, expect_len);
    // VERIFY_INSTR2_MEMBYREG_IMMEDIATE(opcode, mem_regno, mem_offset, value, expect_bytes, expect_len);

    // // two operands operations, target is an address pointed by a symbol
    // VERIFY_INSTR2_MEMBYSYM_REGISTER(opcode, symbol_name, source_regno, expect_bytes, expect_len);
    // VERIFY_INSTR2_MEMBYSYM_IMMEDIATE(opcode, symbol_name, value, expect_bytes, expect_len);

    // if we got here, no test failed
    printf(" OK\n");
}


static void test_full_io() {
    // we should really do this one: https://www.tutorialspoint.com/assembly_programming/assembly_system_calls.htm
    // equivalent to "main() { exit(8); }"
    // 401005:	b8 01 00 00 00       	mov    $0x1,%eax
    // 40100a:	bb 08 00 00 00       	mov    $0x8,%ebx
    // 40100f:	cd 80                	int    $0x80
}

static bool verify_single_instruction(struct instruction *instr, char *expected_bytes, int expected_len) {
    char buff[128];

    instruction_to_string(instr, buff);
    struct x86_encoder *enc = new_x86_encoder(CPU_MODE_PROTECTED);

    if (!enc->encode(enc, instr)) {
        printf("\n");
        printf("  Could not encode instruction '%s'\n", buff);
        enc->free(enc);
        return false;
    }

    if (memcmp(enc->output->data, expected_bytes, expected_len) != 0) {
        printf("\n");
        printf("  Bad instruction encoding '%s'\n", buff);
        printf("  Expected:");
        for (int i = 0; i < expected_len; i++)
            printf(" %02x", (u8)expected_bytes[i]);
        printf("\n");
        printf("  Produced:");
        for (int i = 0; i < enc->output->length; i++)
            printf(" %02x", (u8)enc->output->data[i]);
        printf("\n");
        enc->free(enc);
        return false;
    }

    enc->free(enc);
    printf(".");
    return true;
}

static void verify_listing(char *title, struct instruction *list, int instr_count, char *expected, int expected_len) {
    bool encoded;
    char buff[128];

    printf("Verify listing '%s'...", title);

    struct x86_encoder *encoder = new_x86_encoder(CPU_MODE_PROTECTED);
    for (int i = 0; i < instr_count; i++) {
        encoded = encoder->encode(encoder, &list[i]);
        if (!encoded) {
            instruction_to_string(&list[i], buff);
            printf("Cannot encode expression: \'%s\'\n", buff);
            return;
        }
    }

    if (memcmp(encoder->output->data, expected, expected_len) == 0) {
        printf(" [OK]\n");
        return;
    }

    printf(" [FAILED]\n");

    printf("Assembly code listing:\n");
    for (int i = 0; i < instr_count; i++) {
        instruction_to_string(&list[i], buff);
        printf("\t%s\n", buff);
    }

    printf("Encoded machine code: (%d bytes)\n", encoder->output->length);
    print_16_hex(encoder->output->data, encoder->output->length);

    printf("Unresolved references:\n");
    printf("  Position  Symbol name\n");
    //        12345678  abcdef
    for (int i = 0; i < encoder->relocations->length; i++) {
        struct relocation *r = &encoder->relocations->list[i];
        printf("  %8ld  %s\n", r->position, r->name);
    }

    printf("Expected: ");
    for (int i = 0; i < expected_len; i++)
        printf(" %02x", (unsigned char)expected[i]);
    printf("\n");

    printf("Produced: ");
    for (int i = 0; i < encoder->output->length; i++)
        printf(" %02x", (unsigned char)encoder->output->data[i]);
    printf("\n");
}



#define MOV_REG_IMM(reg, val) \
    listing[count].opcode = OC_MOV;         \
    listing[count].op1.type = OT_REGISTER;  \
    listing[count].op1.value = reg;         \
    listing[count].op2.type = OT_IMMEDIATE; \
    listing[count].op2.value = val;         \
    count++;

#define MOV_REG_SYM(reg, sym) \
    listing[count].opcode = OC_MOV;                   \
    listing[count].op1.type = OT_REGISTER;            \
    listing[count].op1.value = reg;                   \
    listing[count].op2.type = OT_SYMBOL_MEM_ADDRESS;  \
    listing[count].op2.symbol_name = sym;             \
    count++;

#define INT(no) \
    listing[count].opcode = OC_INT;          \
    listing[count].op1.type = OT_IMMEDIATE;  \
    listing[count].op1.value = no;           \
    count++;

void test_create_executable() {
    // based on this: https://www.tutorialspoint.com/assembly_programming/assembly_system_calls.htm

    struct instruction listing[30];
    int count = 0;

    memset(&listing, 0, sizeof(listing));
    // hello_msg,
    // hello_msg_len,
    
    // prepare a data segment as well, keeping address of symbols
    struct bin_buffer *data_seg = new_bin_buffer();
    struct symbol_table *data_symbols = new_symbol_table();
    char *msg = "Hello world!";
    data_symbols->append_symbol(data_symbols, "hello_msg", data_seg->length);
    data_seg->add_strz(data_seg, msg);
    data_symbols->append_symbol(data_symbols, "hello_msg_len", data_seg->length);
    data_seg->add_dword(data_seg, strlen(msg));

    // syscall for write(), eax=4, ebx=handle, ecx=buffer, edx=length
    MOV_REG_IMM(REG_AX, 4);
    MOV_REG_IMM(REG_BX, 1);
    MOV_REG_SYM(REG_CX, "hello_msg");
    MOV_REG_SYM(REG_DX, "hello_msg_len");
    INT(0x80);

    // syscall for exit(), eax=1, ebx=exit_code
    MOV_REG_IMM(REG_AX, 1);
    MOV_REG_IMM(REG_BX, 0);
    INT(0x80);

    // encode this into intel machine code
    struct x86_encoder *enc = new_x86_encoder(CPU_MODE_PROTECTED);
    for (int i = 0; i < count; i++) {
        if (!enc->encode(enc, &listing[i])) {
            char str[128];
            instruction_to_string(&listing[i], str);
            printf("Failed encoding instruction: '%s'\n", str);
            return;
        }
    }
    
    // backfill symbol references
    // we should have at least three tables with three base addresses: .text, .data, .bss
    u64 code_seg_address = 0x8048000;
    u64 data_seg_address = code_seg_address + round_up(enc->output->length, 4096);
    enc->relocations->backfill_buffer(enc->relocations,
        data_symbols, enc->output, data_seg_address);


    // now we should be good. let's write this.
    binary_program *prog = malloc(sizeof(binary_program));
    memset(prog, 0, sizeof(binary_program));

    // .text
    prog->code_address = code_seg_address; // usual starting address
    prog->code_contents = enc->output->data;
    prog->code_size = enc->output->length;
    prog->code_entry_point = code_seg_address; // address of _start, actually...
    // .data
    prog->init_data_address = data_seg_address;
    prog->init_data_contents = data_seg->data;
    prog->init_data_size = data_seg->length;
    // .bss
    prog->zero_data_address = round_up(prog->init_data_address + prog->init_data_size, 4096);
    prog->zero_data_size = 0;
    // flags
    prog->flags.is_64_bits = false;
    prog->flags.is_static_executable = true;

    long elf_size = 0;
    if (!write_elf_file(prog, "out.elf", &elf_size))
        printf("Error writing output elf file!\n");
    else
        printf("Wrote %ld bytes to out.elf file\n", elf_size);
}