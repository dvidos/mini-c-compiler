#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../utils.h"
#include "../run_info.h"
#include "../linker/symbol_table.h"
#include "../linker/obj_code.h"
#include "encoder/encoder.h"
#include "asm_line.h"
#include "asm_listing.h"
#include "assembler.h"
#include "../linker/linker.h"
#include "../elf/elf_contents.h"
#include "../elf/elf64_contents.h"

// static void test_create_hello_world_executable();
static void test_create_hello_world_executable2();
static void test_create_hello_world_executable3();
static bool verify_instructions();
static bool verify_single_instruction(enum opcode oc, asm_operand *op1, asm_operand *op2, char *expected_bytes, int expected_len);




void perform_asm_test() {
    if (!verify_instructions())
        return;
    
    // test_create_hello_world_executable();
    // test_create_hello_world_executable2();
    test_create_hello_world_executable3();
}

#define VERIFY_INSTR0(oc, expect_bytes, expect_len)   \
    if (!verify_single_instruction(oc, NULL, NULL, expect_bytes, expect_len)) return false;

#define VERIFY_INSTR1_IMMEDIATE(code, val, expect_bytes, expect_len) \
    if (!verify_single_instruction(code, new_asm_operand_imm(val), NULL, expect_bytes, expect_len)) return false;

#define VERIFY_INSTR1_REGISTER(code, reg_no, expect_bytes, expect_len) \
    if (!verify_single_instruction(code, new_asm_operand_reg(reg_no), NULL, expect_bytes, expect_len)) return false;

#define VERIFY_INSTR1_MEMBYREG(code, reg_no, offs, expect_bytes, expect_len) \
    if (!verify_single_instruction(code, new_asm_operand_mem_by_reg(reg_no, offs), NULL, expect_bytes, expect_len)) return false;

#define VERIFY_INSTR1_MEMBYSYM(code, sym, expect_bytes, expect_len) \
    if (!verify_single_instruction(code, new_asm_operand_mem_by_sym(sym), NULL, expect_bytes, expect_len)) return false;

#define VERIFY_INSTR2_REG_REG(code, target_regno, source_regno, expect_bytes, expect_len) \
    if (!verify_single_instruction(code, new_asm_operand_reg(target_regno), new_asm_operand_reg(source_regno), expect_bytes, expect_len)) return false;

#define VERIFY_INSTR2_REG_IMMEDIATE(code, regno, val, expect_bytes, expect_len) \
    if (!verify_single_instruction(code, new_asm_operand_reg(regno), new_asm_operand_imm(val), expect_bytes, expect_len)) return false;


static bool verify_instructions() {
    asm_instruction *instr;
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
    VERIFY_INSTR1_REGISTER(OC_PUSH, REG_AX, "\xFF\xF0", 2);
    VERIFY_INSTR1_REGISTER(OC_PUSH, REG_DX, "\xFF\xF2", 2);
    VERIFY_INSTR1_REGISTER(OC_POP,  REG_BX, "\x8F\xC3", 2);
    VERIFY_INSTR1_REGISTER(OC_POP,  REG_CX, "\x8F\xC1", 2);
    VERIFY_INSTR1_REGISTER(OC_INC,  REG_SI, "\xFF\xC6", 2);
    VERIFY_INSTR1_REGISTER(OC_INC,  REG_DI, "\xFF\xC7", 2);
    VERIFY_INSTR1_REGISTER(OC_DEC,  REG_SI, "\xFF\xCE", 2);
    VERIFY_INSTR1_REGISTER(OC_DEC,  REG_DI, "\xFF\xCF", 2);
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
    VERIFY_INSTR1_MEMBYSYM(OC_CALL, "func1", "\xe8\x00\x00\x00\x00", 5);

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

    VERIFY_INSTR2_REG_IMMEDIATE(OC_MOV, REG_DX, 0x0,        "\xC7\xC2\x00\x00\x00\x00", 6);
    VERIFY_INSTR2_REG_IMMEDIATE(OC_MOV, REG_DX, 0x1,        "\xC7\xC2\x01\x00\x00\x00", 6);
    VERIFY_INSTR2_REG_IMMEDIATE(OC_MOV, REG_DX, 0x12345678, "\xC7\xC2\x78\x56\x34\x12", 5);
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
    return true;
}

static bool verify_single_instruction(enum opcode oc, asm_operand *op1, asm_operand *op2, char *expected_bytes, int expected_len) {
    mempool *mp = new_mempool();

    asm_instruction *instr;
    if (op1 == NULL && op2 == NULL)
        instr = new_asm_instruction(oc);
    else if (op1 != NULL && op2 == NULL)
        instr = new_asm_instruction_with_operand(oc, op1);
    else if (op1 != NULL && op2 != NULL)
        instr = new_asm_instruction_with_operands(oc, op1, op2);

    str *s = new_str(mp, NULL);
    asm_instruction_to_str(instr, s, false);

    bin *b = new_bin(mp);
    reloc_list *relocs = new_reloc_list();
    x86_encoder *enc = new_x86_encoder(mp, b, relocs);
    if (!enc->encode_v4(enc, instr)) {
        printf("  Could not encode instruction '%s'\n", str_charptr(s));
        enc->free(enc);
        return false;
    }

    bin *expected = new_bin_from_mem(mp, expected_bytes, expected_len);
    if (bin_cmp(b, expected) != 0) {
        printf("\n");
        printf("  Bad instruction encoding '%s'\n", str_charptr(s));
        printf("  Expected:");
        bin_print_hex(expected, 0, 0, -1, stdout);
        printf("\n");
        printf("  Produced:");
        bin_print_hex(b, 0, 0, -1, stdout);
        printf("\n");
        return false;
    }

    printf(".");
    mempool_release(mp);
    return true;
}


// #define MOV_REG_IMM(reg, val) \
//     asm_listing[count].opcode = OC_MOV;         \
//     asm_listing[count].op1 = new_asm_operand_reg(reg); \
//     asm_listing[count].op2 = new_asm_operand_imm(val); \
//     count++;

// #define MOV_REG_SYM(reg, sym) \
//     asm_listing[count].opcode = OC_MOV;                   \
//     asm_listing[count].op1 = new_asm_operand_reg(reg); \
//     asm_listing[count].op2 = new_asm_operand_mem_by_sym(sym); \
//     count++;

// #define INT(no) \
//     asm_listing[count].opcode = OC_INT;               \
//     asm_listing[count].op1 = new_asm_operand_imm(no); \
//     asm_listing[count].op2 = NULL;                    \
//     count++;

// void test_create_hello_world_executable() {
//     // based on this: https://www.tutorialspoint.com/assembly_programming/assembly_system_calls.htm

//     struct asm_instruction_old asm_listing[30];
//     int count = 0;

//     memset(&asm_listing, 0, sizeof(asm_listing));
    
//     // prepare a data segment as well, keeping address of symbols
//     buffer *data_seg = new_buffer();
//     symbol_table *data_symbols = new_symbol_table();
//     char *msg = "Hello world!\n";
//     data_symbols->add(data_symbols, "hello_msg", data_seg->length, SB_DATA);
//     data_seg->add_strz(data_seg, msg);
//     data_symbols->add(data_symbols, "hello_msg_len", data_seg->length, SB_DATA);
//     data_seg->add_dword(data_seg, strlen(msg));

//     // syscall for write(), eax=4, ebx=handle, ecx=buffer, edx=length
//     MOV_REG_IMM(REG_AX, 4);
//     MOV_REG_IMM(REG_BX, 1);
//     MOV_REG_SYM(REG_CX, "hello_msg");
//     MOV_REG_IMM(REG_DX, 13);
//     INT(0x80);

//     // syscall for exit(), eax=1, ebx=exit_code
//     MOV_REG_IMM(REG_AX, 1);
//     MOV_REG_IMM(REG_BX, 0);
//     INT(0x80);

//     // encode this into intel machine code
//     struct x86_encoder *enc = new_x86_encoder(CPU_MODE_PROTECTED, new_buffer(), new_reloc_list());
//     for (int i = 0; i < count; i++) {
//         if (!enc->encode_old(enc, &asm_listing[i])) {
//             char str[128];
//             instruction_old_to_string(&asm_listing[i], str, sizeof(str));
//             printf("Failed encoding instruction: '%s'\n", str);
//             return;
//         }
//     }
    
//     u64 code_seg_address = 0x8049000; // 0x8048000;
//     u64 data_seg_address = code_seg_address + round_up(enc->output->length, 4096);

//     // backfill relocations
//     enc->relocations->backfill_buffer(enc->relocations,
//         data_symbols, enc->output, code_seg_address, data_seg_address, 0);

//     // now we should be good. let's write this.
//     elf_contents *prog = malloc(sizeof(elf_contents));
//     memset(prog, 0, sizeof(elf_contents));

//     // .text
//     prog->code_address = code_seg_address; // usual starting address
//     prog->code_contents = enc->output->buffer;
//     prog->code_size = enc->output->length;
//     prog->code_entry_point = code_seg_address; // address of _start, actually...
//     // .data
//     prog->data_address = data_seg_address;
//     prog->data_contents = data_seg->buffer;
//     prog->data_size = data_seg->length;
//     // .bss
//     prog->bss_address = round_up(prog->data_address + prog->data_size, 4096);
//     prog->bss_size = 0;
//     // flags
//     prog->flags.is_64_bits = false;
//     prog->flags.is_static_executable = true;

//     // seems to have been encoded correctly, despite the seg fault
//     /*
//         $ objdump -d out.elf

//         out.elf:     file format elf32-i386
//         Disassembly of section .text:
//         08048000 <.text>:

//         8048000:	b8 04 00 00 00       	mov    $0x4,%eax
//         8048005:	bb 01 00 00 00       	mov    $0x1,%ebx
//         804800a:	b9 00 90 04 08       	mov    $0x8049000,%ecx
//         804800f:	ba 0d 90 04 08       	mov    $0x804900d,%edx
//         8048014:	cd 80                	int    $0x80
//         8048016:	b8 01 00 00 00       	mov    $0x1,%eax
//         804801b:	bb 00 00 00 00       	mov    $0x0,%ebx
//         8048020:	cd 80                	int    $0x80
//     */

//     long elf_size = 0;
//     if (!write_elf_file(prog, "out.elf", &elf_size))
//         printf("Error writing output elf file!\n");
//     else
//         printf("Wrote %ld bytes to out.elf file\n", elf_size);
// }


static bool _test_encode_listing_code(asm_listing *lst, obj_code *mod);
static bool _test_link_module(obj_code *mod, u64 code_base_address, char *filename);

void test_create_hello_world_executable2() {
    mempool *mp = new_mempool();
    asm_listing *lst = new_asm_listing(mp);
    obj_code *mod = new_obj_code();
    
    mod->vt->declare_data(mod, "hello_msg", 13 + 1, "Hello World!\n");

    lst->ops->set_next_label(lst, "_start");
    lst->ops->add_instruction(lst, new_asm_instruction_with_operands(OC_MOV, new_asm_operand_reg(REG_AX), new_asm_operand_imm(4)));
    lst->ops->add_instruction(lst, new_asm_instruction_with_operands(OC_MOV, new_asm_operand_reg(REG_BX), new_asm_operand_imm(1)));
    lst->ops->add_instruction(lst, new_asm_instruction_with_operands(OC_MOV, new_asm_operand_reg(REG_CX), new_asm_operand_mem_by_sym("hello_msg")));
    lst->ops->add_instruction(lst, new_asm_instruction_with_operands(OC_MOV, new_asm_operand_reg(REG_DX), new_asm_operand_imm(13)));
    lst->ops->add_instruction(lst, new_asm_instruction_with_operand(OC_INT, new_asm_operand_imm(0x80)));

    lst->ops->set_next_label(lst, "_exit");
    lst->ops->add_instruction(lst, new_asm_instruction_with_operands(OC_MOV, new_asm_operand_reg(REG_AX), new_asm_operand_imm(1)));
    lst->ops->add_instruction(lst, new_asm_instruction_with_operands(OC_MOV, new_asm_operand_reg(REG_BX), new_asm_operand_imm(0)));
    lst->ops->add_instruction(lst, new_asm_instruction_with_operand(OC_INT, new_asm_operand_imm(0x80)));

    lst->ops->print(lst, stdout);

    // now we need an assembler to convert the asm_listing into a mod code + labels + relocs
    if (!_test_encode_listing_code(lst, mod))
        return;

    printf("Prepared obj_code:\n");
    mod->vt->print(mod);
    
    // now we need a linker to convert the obj_code into an executable
    if (!_test_link_module(mod, 0x8049000, "out2.elf"))
        return;
}

static bool _test_encode_listing_code(asm_listing *lst, obj_code *mod) {
    // encode this into intel machine code
    mempool *mp = new_mempool();
    struct x86_encoder *enc = new_x86_encoder(mp, mod->text->contents, mod->text->relocations);
    struct asm_instruction *inst;

    for_list(lst->lines, asm_line, line) {
        inst = line->per_type.instruction;

        if (line->label != NULL) {
            // we don't know if this is exported for now
            mod->text->symbols->add(mod->text->symbols, str_charptr(line->label), 
                bin_len(mod->text->contents), 0, ST_FUNCTION, false);
        }

        if (!enc->encode_v4(enc, inst)) {
            str *s = new_str(mp, NULL);
            asm_instruction_to_str(inst, s, false);
            printf("Failed encoding instruction: '%s'\n", str_charptr(s));
            mempool_release(mp);
            return false;
        }
    }

    return true;
}

static bool _test_link_module(obj_code *mod, u64 code_base_address, char *filename) {
    // in theory many modules, merge them together, update references and symbol addresses
    // map: code, then data, then bss
    // decide base addresses, resolve references.
    // find "_start" as entry point set it
    // save executable

    struct symbol_entry *start = mod->text->symbols->find(mod->text->symbols, "_start");
    if (start == NULL) {
        printf("Entry point '_start' not found\n");
        return false;
    }
    
    u64 data_base_address;
    u64 bss_base_address;
    u64 rodata_base_address;

    data_base_address = round_up(code_base_address + bin_len(mod->text->contents), 4096);
    bss_base_address = round_up(data_base_address + bin_len(mod->data->contents), 4096);
    bss_base_address = round_up(bss_base_address + bin_len(mod->bss->contents), 4096);

    mod->text->symbols->offset(mod->text->symbols, code_base_address);
    mod->data->symbols->offset(mod->data->symbols, data_base_address);
    mod->bss->symbols->offset(mod->bss->symbols, bss_base_address);
    mod->rodata->symbols->offset(mod->rodata->symbols, rodata_base_address);

    if (!mod->text->relocations->backfill_buffer(mod->text->relocations, mod->text->symbols, mod->text->contents)) {
        printf("Error resolving references\n");
        return false;
    }

    // let's save things
    elf_contents elf;
    elf.flags.is_static_executable = true;
    elf.flags.is_64_bits = false;
    elf.code_address = code_base_address; // usual starting address
    elf.code_contents = bin_ptr_at(mod->text->contents, 0);
    elf.code_size = bin_len(mod->text->contents);
    elf.code_entry_point = code_base_address + start->address; // address of _start, actually...
    elf.data_address = data_base_address;
    elf.data_contents = bin_ptr_at(mod->data->contents, 0);
    elf.data_size = bin_len(mod->data->contents);
    elf.bss_address = bss_base_address;
    elf.bss_size = bin_len(mod->bss->contents);
    
    bool write_elf_file(elf_contents *prog, char *filename);
    if (!write_elf_file(&elf, filename)) {
        printf("Error writing output elf file!\n");
        return false;
    }

    printf("Wrote output to '%s'\n", filename);
}

static void test_create_hello_world_executable3() {

    mempool *mp = new_mempool();

    asm_listing *l = new_asm_listing(mp);
    l->ops->set_next_label(l, "_start");
    l->ops->add_instruction(l, new_asm_instruction_with_operands(OC_MOV, new_asm_operand_reg(REG_AX), new_asm_operand_imm(4)));
    l->ops->add_instruction(l, new_asm_instruction_with_operands(OC_MOV, new_asm_operand_reg(REG_BX), new_asm_operand_imm(1)));
    l->ops->add_instruction(l, new_asm_instruction_with_operands(OC_MOV, new_asm_operand_reg(REG_CX), new_asm_operand_mem_by_sym("hello_msg")));
    l->ops->add_instruction(l, new_asm_instruction_with_operands(OC_MOV, new_asm_operand_reg(REG_DX), new_asm_operand_imm(13)));
    l->ops->add_instruction(l, new_asm_instruction_with_operand(OC_INT, new_asm_operand_imm(0x80)));
    l->ops->set_next_label(l, "_exit");
    l->ops->add_instruction(l, new_asm_instruction_with_operands(OC_MOV, new_asm_operand_reg(REG_AX), new_asm_operand_imm(1)));
    l->ops->add_instruction(l, new_asm_instruction_with_operands(OC_MOV, new_asm_operand_reg(REG_BX), new_asm_operand_imm(0)));
    l->ops->add_instruction(l, new_asm_instruction_with_operand(OC_INT, new_asm_operand_imm(0x80)));

    obj_code *c = new_obj_code();
    c->vt->declare_data(c, "hello_msg", 13 + 1, "Hello World!\n");
    assemble_listing_into_i386_code(mp, l, c);

    // link into executable
    // list *modules = new_list();
    // modules->v->add(modules, c);
    // x86_link(modules, 0x8048000, "hello.elf");
}
