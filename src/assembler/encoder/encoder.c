#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../../err_handler.h"
#include "../../linker/obj_code.h"
#include "encoder.h"
#include "asm_listing.h"
#include "encoded_instruction.h"
#include "encoding_info.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


/*
    machine code directly in the executable .text segment
    | .global _start
    | _start:
    |     MOV $0x12345, %EAX    
    $ as -o tiny.obj tiny.S && ld tiny.obj
    $ objdump -xd a.out
    | a.out:     file format elf64-x86-64
    | Architecture: i386:x86-64, flags 0x00000112: EXEC_P, HAS_SYMS, D_PAGED
    | Start address 0x0000000000401000
    | Program Header:
    |     LOAD off    0x0000000000000000 vaddr 0x0000000000400000 paddr 0x0000000000400000 align 2**12
    |          filesz 0x00000000000000b0 memsz 0x00000000000000b0 flags r--
    |     LOAD off    0x0000000000001000 vaddr 0x0000000000401000 paddr 0x0000000000401000 align 2**12
    |          filesz 0x0000000000000005 memsz 0x0000000000000005 flags r-x
    | Sections:
    | Idx Name          Size      VMA               LMA               File off  Algn
    |   0 .text         00000005  0000000000401000  0000000000401000  00001000  2**0
    |                   CONTENTS, ALLOC, LOAD, READONLY, CODE
    | SYMBOL TABLE:
    | 0000000000401000 g       .text	0000000000000000 _start
    | Disassembly of section .text:
    | 0000000000401000 <_start>:
    |   401000:	b8 45 23 01 00       	mov    $0x12345,%eax
    $ ./a.out
    Segmentation fault (core dumped)    :-)
    (but if I make an infinite loop, it runs without crashing!)
*/



/*
    Intel's manual is crystal clear, but endless!
    an empirical approach here: https://defuse.ca/online-x86-assembler.htm#disassembly
    also good assembly intro: https://www.cs.virginia.edu/~evans/cs216/guides/x86.html
    and good ref:             https://www.felixcloutier.com/x86/
    even more concise: https://wiki.osdev.org/X86-64_Instruction_Encoding#Registers



    for example:
        addr  machine code      assembly
        0:    50                push   eax
        1:    51                push   ecx
        2:    52                push   edx
        3:    53                push   ebx
        4:    54                push   esp
        5:    55                push   ebp
        6:    56                push   esi
        7:    57                push   edi
        8:    58                pop    eax
        9:    59                pop    ecx
        a:    5a                pop    edx
        b:    5b                pop    ebx
        c:    5c                pop    esp
        d:    5d                pop    ebp
        e:    5e                pop    esi
        f:    5f                pop    edi
        00000010 <test>:
        10:   eb fe             jmp    10 <test>
        12:   c3                ret
        13:   e8 f8 ff ff ff    call   10 <test>
        18:   39 d8             cmp    eax,ebx
        1a:   39 c8             cmp    eax,ecx
        1c:   39 d0             cmp    eax,edx

        jmp near = EB + relative signed byte (+/-127)
        jmp far  = E9 + four bytes of relative address (e.g. +4k)

*/

/*
    registers: EAX (AX, AL, AH), EBX, ECX, EDX, ESP, EBP, ESI, EDI
    data defined using ".data" and then a name, followed by "db", "dw", "dd" (1, 2, 4 bytes)
    and a number of how many to allocate, or a string with a ",0" for a db.
    ----
    the things to go with an opcde seem to be:
    - addressing mode (register, address, number, reg+offset etc)
    - when using a pointer, size of pointed thing (1, 2, 4 bytes)
    - which register is to affect (e.g. PUSH EAX vs PUSH EBX)
    ----
    minimum set of opcodes to support:
    MOV, PUSH, POP, LEA, 
    ADD, SUB, INC, DEC, IMUL, IDIV, AND, OR, XOR, NOT, NEG (two's complement), SHL, SHR,
    JMP, J<condition>, CMP, CALL, RET
    -----
    seems each instruction consists of:
    - optional 1 byte prefix (e.g. REPNZ etc)
    - 1,2,3 bytes opcode
    - a possible ModR/M addressing mode byte (2-3-3 bits, see Intel manual 2, page 44)
        - 2 bits mod field (addressing modes, 00=no disp, 01=8bit disp, 10=32bit displacement)
        - 3 bits reg/opcode field (either register, or extra opcode info, depends on opcode)
            roughly [EAX], [ECX], [EDX], [EBX], (SIB), [EBP], [ESI], [EDI]
        - 3 bits r/m field (register or combined with mod)
            roughly EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    - a possible scale-index-base byte (SIB, 2-3-3 bits)
        - 2 bits scale factor (00=1, 01=*2, 10=*4, 11=*8)
        - 3 bits scaled index (EAX*scale, ECX*scale, EDX*scale, EBX*scale, -, EBP*scale, ESI*scale, EDI*scale)
        - 3 bits base (EAX, ECX, EDX, EBX, ESP, -, ESI, EDI)
    - a possible address displacement of 1, 2, 4 bytes
    - a possible immediate data of 1, 2, 4 bytes
*/


// intermediate language:
// CALL, JUMP_IF<cond>, SET, RET a = b <op> c
// <cond> = [EQ, NE, GT, GE, LT, LE, T, F]
// <op> = ADD, SUB, MUL, DIV, AND, OR, XOR, NOT, NEG, SHL, SHR

// Disassembly of section .text:  00000000 <.text>:  0:	b8 34 12 00 00       	mov    $0x1234,%eax
// Disassembly of section .text:  00000000 <.text>:  0:	b9 34 12 00 00       	mov    $0x1234,%ecx
// MOV for 32 bits, B8+, meaning B8+ number of processor, similarly to PUSH



// we'd need symbol tables and base addresses for code and data?
// i mean the output is not only the binary data, but the symbol tables as well.
// how shall we encode symbol relocations????
// encoding should allow for referencing symbols, not only final binary code
// we would need a table, that indicates how each instruction is encoded,
// i.e. whether it needs a +r8, or a ModR/M or Immediate etc
// then encoding would be really simple, just branching with
// the different ways to encode instructions.

/*
    Having implemented the first executable, I think we can now take a better look at the 
    instruction encoding, especially since we have not taken into account the memory sizes.
    Meaning, we hardcoded the memory loading and storing to be done with dwords only.
    I think we must take this into account and any instruction must understand (or have hint)
    at the size of the operation (e.g. 8, 16, 32 or 64 bits)
    Then we shall have different encoding for each of these widths.

    Cases (each of the cases has 4 encodings, due to size of 8/16/32/64 bits)
    Also, memory can be accessed via register +/- offset or by direct address (symbols)

    PUSH    REG
    PUSH    IMM  ; size may be needed, i.e. push how many bytes???
    PUSH    MEM
    MOV     REG     REG
    MOV     REG     MEM
    MOV     MEM     REG
    MOV     MEM     IMM   ; size hint needed
    MOV     REG     IMM

    By looking at the source code of nasm I understand that my 'listing'
    is a type of contract of what we can encode. For nasm, this is the user,
    for us, it seems I want to offer a type of generalization on top of the opcodes.
    e.g. in my approach, there's no way to load CS, SS or LGTR
    
    Nasm does it by an instruction table, out of which, code is generated.
    https://github.com/netwide-assembler/nasm/blob/master/x86/insns.dat

*/


static bool _encode_old(struct x86_encoder *enc, struct asm_instruction_old *instr);
static void _reset(struct x86_encoder *enc);
static void _free(struct x86_encoder *enc);

struct x86_encoder *new_x86_encoder(enum x86_cpu_mode mode, buffer *code_out, reloc_list *relocations_out) {
    struct x86_encoder *enc = malloc(sizeof(struct x86_encoder));
    enc->mode = mode;

    enc->output = code_out;
    enc->relocations = relocations_out;

    enc->encode_old = _encode_old;
    enc->reset = _reset;
    enc->free = _free;
};

// seems like encoding is heavily based on the operands type + size.
// e.g. simple push is 50+reg
//      push with 32 bit value is 68 + value
//      push with 32 bit ptr   is ff 35 + ptr
// or:  add r/m imm32 is 81 (type of second operand affects)
//      add r/m r32   is 01 (type of second operand affects)
// lea reg, mem
// etc. good investigation here https://www.cs.virginia.edu/~evans/cs216/guides/x86.html

// - instructions with ModR/M: the 'reg' and 'r/m' fields of the byte
// - instructions with ModR/M + SIB: the 'reg' of the ModRM, the base+idx+offs fields of the SIB
// - instructions without ModR/M: the reg field in the opcode
// - in 64 bits, REX prefixes can provide fourth bits to various fields

// in intel's documentation, opcode notation:
// - /digit : put this digit in the 'reg' field of ModRM, use the 'r/m' field for reg or memory.
// - /r     : the ModRM contains both a register operand and a r/m operand
// - cb,cb,cw,cd,cp,co,ct : A 1,2,4,6,8,10 bytes value following the opcode, a code offset
// - ib,iw,id,io          : A 1,2,4,8 bytes value following opcode/mod/sib bytes.
// - +rb,+rw,+rd,+ro      : The register identifier in the lower 3 bits of the opcode, no ModRM is needed
// in intel's documentation, encoding info:
// - rel8,rel16,rel32 : A relative address. 8bits is signed, the others are CS relative
// - r8,r16,r32,r64   : One of the general purpose registers.
// - imm8,imm16,32,64 : An immediate value, signed.
// - r/m8,r/m16,32,64 : A register or memory address with the relevant size.


// example of ModR/M:
// assembly                            machine code             mod reg r/m
// ------------------------------------------------------------------------
// mov    eax,edi                      89 f8                  f8=11 111 000
// mov    ecx,edi                      89 f9                  f9=11 111 001
// mov    edx,edi                      89 fa                  fa=11 111 010
// mov    ebx,edi                      89 fb                  fb=11 111 011
// ------------------------------------------------------------------------
// mov    edi,eax                      89 c7                  c7=11 000 111
// mov    edi,ecx                      89 cf                  cf=11 001 111
// mov    edi,edx                      89 d7                  d7=11 010 111
// mov    edi,ebx                      89 df                  df=11 011 111
// ------------------------------------------------------------------------
// mov    eax,DWORD PTR [esi]          8b 06                  06=00 000 110
// mov    eax,DWORD PTR [edi]          8b 07                  07=00 000 111
// mov    eax,DWORD PTR [edi+0x2]      8b 47 02               47=01 000 111
// mov    eax,DWORD PTR [edi-0x2]      8b 47 fe               47=01 000 111  fe=(-2)
// mov    eax,DWORD PTR [edi+0xffff]   8b 87 ff ff 00 00      87=10 000 111
// ------------------------------------------------------------------------
// mov    DWORD PTR [esi],eax          89 06                  06=00 000 110
// mov    DWORD PTR [edi],eax          89 07                  07=00 000 111
// mov    DWORD PTR [edi+0x2],eax      89 47 02               47=01 000 111
// mov    DWORD PTR [edi-0x2],eax      89 47 fe               47=01 000 111
// mov    DWORD PTR [edi+0xffff],eax   89 87 ff ff 00 00      87=10 000 111
// ----------------------------------------------------------------------
// mov    eax,0x1                      b8 01 00 00 00         b8+ immediate
// mov    eax,0xffff                   b8 ff ff 00 00            
// mov    eax,0x12345678               b8 78 56 34 12            
// mov    BYTE PTR [eax],0x12          c6 00 12               00=00 000 000  imm=12
// mov    WORD PTR [eax],0x123         66 c7 00 23 01         c7=11 000 111  (66? should be c7?) 
// mov    DWORD PTR [eax],0x123        c7 00 23 01 00 00      00=00 000 000
// mov    DWORD PTR [eax+0x1],0x123    c7 40 01 23 01 00 00   40=10 000 000  01=offset
// ----------------------------------------------------------------------
// so, mov imm -> mem, address by register +/-offset, width by opcdode, value follows
// ----------------------------------------------------------------------

struct encoding_information {
    enum opcode opcode;
    u8 num_operands;
    u8 inst_byte;
    bool add_regno_to_base_opcode; // eg the "+ rw" to add reg num to base opcode
    u8 opcode_extension;         // eg the "/6" extension of the opcode (0-7 valid only)
};

enum modrm_mode {
    MODE_INDIRECT_NO_DISPLACEMENT         = 0x0,
    MODE_INDIRECT_ONE_BYTE_DISPLACEMENT   = 0x1,
    MODE_INDIRECT_FOUR_BYTES_DISPLACEMENT = 0x2,
    MODE_DIRECT_REGISTER                  = 0x3
};

static inline u8 modrm_byte(int mode, int reg, int reg_mem);
static inline u8 sib_byte(int scale, int index, int base);
static bool encode_single_byte_instruction_adding_reg_no(struct x86_encoder *enc, u8 base_opcode, int reg_no);
static bool encode_ext_instr_direct_reg(struct x86_encoder *enc, u8 opcode, u8 ext_opcode, int reg_no);
static bool encode_ext_instr_mem_by_register(struct x86_encoder *enc, u8 opcode, u8 ext_opcode, int reg_no, int displacement);
static bool encode_ext_instr_mem_by_symbol(struct x86_encoder *enc, u8 opcode, u8 ext_opcode, char *symbol_name);



static bool _encode_old(struct x86_encoder *enc, struct asm_instruction_old *instr) {
    if (enc->mode != CPU_MODE_PROTECTED) {
        return false;
    }

    switch (instr->opcode) {
        case OC_NOP:
            enc->output->add_byte(enc->output, 0x90); // simplest case
            break;

        case OC_PUSH:
            if (instr->op1->type == OT_REGISTER) {
                // this one cannot push segment registers, only general ones
                return encode_single_byte_instruction_adding_reg_no(enc, 0x50, instr->op1->reg);

            } else if (instr->op1->type == OT_MEM_POINTED_BY_REG) {
                // Intel gives this as: "FF /6   ModRM:r/m (r)"
                return encode_ext_instr_mem_by_register(enc, 0xFF, 6, instr->op1->reg, instr->op1->offset);

            } else if (instr->op1->type == OT_MEM_OF_SYMBOL) {
                // Intel gives this as: "FF /6   ModRM:r/m (r)"
                return encode_ext_instr_mem_by_symbol(enc, 0xFF, 6, instr->op1->symbol_name);

            } else if (instr->op1->type == OT_IMMEDIATE) {
                // simple 68 + value
                if (instr->op1->immediate >= -128 && instr->op1->immediate <= 127) {
                    enc->output->add_byte(enc->output, 0x6a);
                    enc->output->add_byte(enc->output, (u8)instr->op1->immediate);
                } else {
                    enc->output->add_byte(enc->output, 0x68);
                    enc->output->add_dword(enc->output, instr->op1->immediate);
                }
                return true;
            } else {
                return false;
            }
            break;
        case OC_POP:
            if (instr->op1->type == OT_REGISTER) {
                // this one cannot push segment registers, only general ones
                return encode_single_byte_instruction_adding_reg_no(enc, 0x58, instr->op1->reg);

            } else if (instr->op1->type == OT_MEM_POINTED_BY_REG) {
                // Intel gives this as: "8F /0"
                return encode_ext_instr_mem_by_register(enc, 0x8F, 0, instr->op1->reg, instr->op1->offset);

            } else if (instr->op1->type == OT_MEM_OF_SYMBOL) {
                // Intel gives this as: "8F /6   ModRM:r/m (r)"
                return encode_ext_instr_mem_by_symbol(enc, 0x8F, 0, instr->op1->symbol_name);

            } else {
                return false;
            }
            break;
        case OC_MOV:
            // mov reg, imm    - B8
            // mov reg, reg    - 89 or 8B
            // mov reg, mem    - 8B
            // mov mem, reg    - 89
            // mov mem, imm    - B8 or C7
            if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_IMMEDIATE) {
                // B8+reg, imm32
                enc->output->add_byte(enc->output, 0xB8 + (instr->op1->immediate & 0x7));
                enc->output->add_dword(enc->output, instr->op2->immediate);
            } else if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_REGISTER) {
                // "89 /r"  (mode 11, reg=src, r/m=dest)
                enc->output->add_byte(enc->output, 0x89);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, instr->op2->reg, instr->op1->reg));
            } else if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_MEM_POINTED_BY_REG) {
                // "8B /r"  (mode , reg=src, r/m=dest)
                return encode_ext_instr_mem_by_register(enc, 
                    0x8B, instr->op1->reg, instr->op2->reg, instr->op2->offset);
            } else if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_MEM_OF_SYMBOL) {
                encode_single_byte_instruction_adding_reg_no(enc, 0xB8, instr->op1->reg);
                enc->relocations->add(enc->relocations, enc->output->length, instr->op2->symbol_name, RT_ABS_32);
                enc->output->add_dword(enc->output, 0xFFFFFFFF);
                return true;
                
            } else if (instr->op1->type == OT_MEM_POINTED_BY_REG && instr->op2->type == OT_REGISTER) {

            } else if (instr->op1->type == OT_MEM_OF_SYMBOL && instr->op2->type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0xA3);
                enc->relocations->add(enc->relocations, enc->output->length, instr->op1->symbol_name, RT_ABS_32);
                enc->output->add_dword(enc->output, 0xFFFFFFFF);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 0, instr->op2->reg));
                return true;

            } else if (instr->op1->type == OT_MEM_POINTED_BY_REG && instr->op2->type == OT_IMMEDIATE) {
            } else if (instr->op1->type == OT_MEM_OF_SYMBOL && instr->op2->type == OT_IMMEDIATE) {
            } else {
                return false;
            }
            break;
        case OC_INT:
            // should be easy
            enc->output->add_byte(enc->output, 0xCD);
            enc->output->add_byte(enc->output, instr->op1->immediate);
            return true;
            break;

        case OC_RET:
            // near ret assumes no change in CS
            enc->output->add_byte(enc->output, 0xC3);
            return true;
            break;

        case OC_INC:
            if (instr->op1->type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x40 + (instr->op1->reg & 0x7));
                return true;
            } else if (instr->op1->type == OT_MEM_POINTED_BY_REG) {
                return encode_ext_instr_mem_by_register(enc, 0xFF, 0, instr->op1->reg, instr->op1->offset);
            } else if (instr->op1->type == OT_MEM_OF_SYMBOL) {
                return encode_ext_instr_mem_by_symbol(enc, 0xFF, 0, instr->op1->symbol_name);
            }
            break;
        case OC_DEC:
            if (instr->op1->type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x48 + (instr->op1->reg & 0x7));
                return true;
            } else if (instr->op1->type == OT_MEM_POINTED_BY_REG) {
                return encode_ext_instr_mem_by_register(enc, 0xFF, 1, instr->op1->reg, instr->op1->offset);
            } else if (instr->op1->type == OT_MEM_OF_SYMBOL) {
                return encode_ext_instr_mem_by_symbol(enc, 0xFF, 1, instr->op1->symbol_name);
            }
            break;
        case OC_NEG:
            if (instr->op1->type == OT_REGISTER) {
                // "F7/3"
                enc->output->add_byte(enc->output, 0xF7);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 3, instr->op1->reg));
                return true;
            } else if (instr->op1->type == OT_MEM_POINTED_BY_REG) {
                // "F7/3"
                return encode_ext_instr_mem_by_register(enc, 0xF7, 3, instr->op1->reg, instr->op1->offset);
            } else if (instr->op1->type == OT_MEM_OF_SYMBOL) {
                // "F7/3"
                return encode_ext_instr_mem_by_symbol(enc, 0xF7, 3, instr->op1->symbol_name);
            }
            break;
        case OC_NOT:
            if (instr->op1->type == OT_REGISTER) {
                // "F7/2"
                enc->output->add_byte(enc->output, 0xF7);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 2, instr->op1->reg));
                return true;
            } else if (instr->op1->type == OT_MEM_POINTED_BY_REG) {
                // "F7/2"
                return encode_ext_instr_mem_by_register(enc, 0xF7, 2, instr->op1->reg, instr->op1->offset);
            } else if (instr->op1->type == OT_MEM_OF_SYMBOL) {
                // "F7/2"
                return encode_ext_instr_mem_by_symbol(enc, 0xF7, 2, instr->op1->symbol_name);
            }
            break;
        case OC_CALL:
            if (instr->op1->type == OT_REGISTER) {
                // "FF/2"
                enc->output->add_byte(enc->output, 0xFF);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 2, instr->op1->reg));
                return true;
            } else if (instr->op1->type == OT_MEM_POINTED_BY_REG) {
                // "FF/2"
                return encode_ext_instr_mem_by_register(enc, 0xFF, 2, instr->op1->reg, instr->op1->offset);
            } else if (instr->op1->type == OT_MEM_OF_SYMBOL) {
                // "FF/2", we'll use a CODE_SEG override to make this an absolute call,
                //         otherwise it will be relative to the next instruction
                //         and it'd be encoded as "e8 01 00 00 00" with dword as relative offset
                enc->output->add_byte(enc->output, 0x2E);
                return encode_ext_instr_mem_by_symbol(enc, 0xFF, 2, instr->op1->symbol_name);
            }
            break;
        case OC_ADD:
            if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x01);
                // for the "reg <- reg" cases, we put the destination in "reg" and source in "reg_mom"
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, instr->op2->reg, instr->op1->reg));
                return true;
            } else if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_IMMEDIATE) {
                enc->output->add_byte(enc->output, 0x81);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 0, instr->op1->reg));
                enc->output->add_dword(enc->output, instr->op2->immediate);
                return true;
            }
            break;
        case OC_SUB:
            if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x29);
                // for the "reg <- reg" cases, we put the destination in "reg" and source in "reg_mom"
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, instr->op2->reg, instr->op1->reg));
                return true;
            } else if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_IMMEDIATE) {
                enc->output->add_byte(enc->output, 0x81);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 5, instr->op1->reg));
                enc->output->add_dword(enc->output, instr->op2->immediate);
                return true;
            }
            break;
        case OC_MUL:
            break;
        case OC_DIV:
            break;
        case OC_AND:
            if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x21);
                // for the "reg <- reg" cases, we put the destination in "reg" and source in "reg_mom"
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, instr->op2->reg, instr->op1->reg));
                return true;
            }
            break;
        case OC_OR:
            if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x09);
                // for the "reg <- reg" cases, we put the destination in "reg" and source in "reg_mom"
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, instr->op2->reg, instr->op1->reg));
                return true;
            }
            break;
        case OC_XOR:
            if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x31);
                // for the "reg <- reg" cases, we put the destination in "reg" and source in "reg_mom"
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, instr->op2->reg, instr->op1->reg));
                return true;
            }
            break;
        case OC_SHR:
            if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_IMMEDIATE) {
                // "C1/5 SHR r/m32, imm8"
                enc->output->add_byte(enc->output, 0xC1);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 5, instr->op1->reg));
                enc->output->add_byte(enc->output, instr->op2->immediate);
                return true;
            }
            break;
        case OC_SHL:
            if (instr->op1->type == OT_REGISTER && instr->op2->type == OT_IMMEDIATE) {
                // "C1/4 SHL r/m32, imm8"
                enc->output->add_byte(enc->output, 0xC1);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 4, instr->op1->reg));
                enc->output->add_byte(enc->output, instr->op2->immediate);
                return true;
            }
            break;
        default:
            return false;
    }
    return true;
}


static inline u8 modrm_byte(int mode, int reg, int reg_mem) {
    // 2 bits + 3 bits + 3 bits
    return ((mode & 0x3) << 6) | ((reg & 0x7) << 3) | (reg_mem & 0x7);
}

static inline u8 sib_byte(int scale, int index, int base) {
    // 2 bits + 3 bits + 3 bits
    return ((scale & 0x3) << 6) | ((index & 0x7) << 3) | (base & 0x7);
}

static bool encode_single_byte_instruction_adding_reg_no(struct x86_encoder *enc, u8 base_opcode, int reg_no) {
    // e.g. "PUSH ECX" -> 51
    enc->output->add_byte(enc->output, base_opcode + (reg_no & 0x7));
    return true;
}

static bool encode_ext_instr_direct_reg(struct x86_encoder *enc, u8 opcode, u8 ext_opcode, int reg_no) {
    // e.g. "PUSH EAX"
    // ext_opcode goes to the "reg" field, reg_no is set, 
    enc->output->add_byte(enc->output, opcode);
    enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, ext_opcode, reg_no));
    return true;
}

static bool encode_ext_instr_mem_by_register(struct x86_encoder *enc, u8 opcode, u8 ext_opcode, int reg_no, int displacement) {
    // e.g. "PUSH [EAX+2]"
    // ext_opcode goes to the "reg" field, 
    // we shall code the special cases of SP and BP, 
    // as well as take into account how many bytes of displacement
    // this will tell us the mode bits
    u8 mode;
    u8 reg_mem;

    // ESP cannot be used for memory reference, it's not encodable
    if (reg_no == REG_SP)
        return false;

    if (displacement == 0) {
        mode = MODE_INDIRECT_NO_DISPLACEMENT;
    } else if (displacement >= -128 && displacement <= 127) {
        mode = MODE_INDIRECT_ONE_BYTE_DISPLACEMENT;
    } else {
        mode = MODE_INDIRECT_FOUR_BYTES_DISPLACEMENT;
    }

    // EBP cannot be accessed directly, only through a displacement, we use zero
    if (reg_no == REG_BP && mode == 0) {
        mode = MODE_INDIRECT_ONE_BYTE_DISPLACEMENT;
        displacement = 0;
    }

    // finally...
    enc->output->add_byte(enc->output, opcode);
    enc->output->add_byte(enc->output, modrm_byte(mode, ext_opcode, reg_no));

    if (mode == MODE_INDIRECT_ONE_BYTE_DISPLACEMENT)
        enc->output->add_byte(enc->output, (char)displacement);
    else if (mode == MODE_INDIRECT_FOUR_BYTES_DISPLACEMENT)
        enc->output->add_dword(enc->output, displacement);

    return true;
}

static bool encode_ext_instr_mem_by_symbol(struct x86_encoder *enc, u8 opcode, u8 ext_opcode, char *symbol_name) {
    // we utilize the 00 (bin) mode and 101 (bin) R/M value which actually means direct displacement
    // 0x5 is the R/M value for direct memory addressing
    // e.g. "PUSH [0x1234]" -> FF 35 34 12 00 00       push dword ptr [0x1234]
    enc->output->add_byte(enc->output, opcode);
    enc->output->add_byte(enc->output, modrm_byte(MODE_INDIRECT_NO_DISPLACEMENT, ext_opcode, 0x5));

    // save reference to backfill four bytes
    enc->relocations->add(enc->relocations, enc->output->length, symbol_name, RT_ABS_32);
    enc->output->add_dword(enc->output, 0xFFFFFFFF);
    return true;
}

static void _reset(struct x86_encoder *enc) {
    enc->output->clear(enc->output);
    enc->relocations->clear(enc->relocations);
}

static void _free(struct x86_encoder *enc) {
    enc->output->free(enc->output);
    enc->relocations->free(enc->relocations);
    free(enc);
}

// -----------------------------
// encoder v4 below
// -----------------------------


static bool encode_asm_instr_opcode(asm_instruction *instr, struct encoding_info *info, struct encoded_instruction *result) {

    // setup 16-bit operand size if needed
    if (instr->operands_size_bits == 16) {
        // not sure of difference between operands size and address size...
        result->flags.have_operand_size_prefix = 1;
        result->operand_size_prefix = 0x66;
    }

    // find if instruction requires expansion byte and set it up.
    if (info->has_instruction_expansion_byte) {
        // some instructions have a 0x0F (e.g. BSF)
        result->flags.have_opcode_expansion_byte = 1;
        result->opcode_expansion_byte = info->instruction_expansion_byte;
    }

    result->opcode_byte = info->base_opcode_byte;

    if (info->has_direction_bit) {
        if (instr->direction_op1_to_op2)  // one means from R/M --> Reg in the ModRegR/M byte                
            result->opcode_byte |= 0x2;
        else // zero means from Reg --> R/M in the ModRegR/M byte
            result->opcode_byte &= ~0x2;
    }

    // width bit works both for immediate and for reg-mem types of instructions
    if (info->has_width_bit) {
        if (instr->operands_size_bits == 8) // zero indicates single byte operands
            result->opcode_byte &= ~0x1;
        else // one indicates full size operands
            result->opcode_byte |= 0x1;
    }
    
    // some immediate instructions have this (e.g. "SAL D0 /7")
    if (info->has_opcode_extension) {
        result->flags.have_modregrm = true;
        result->modregrm_byte |= ((info->opcode_extension_value & 0x7) << 3);
    }

    return true;
}

static bool encode_asm_instr_operands(asm_instruction *instr, struct encoding_info *info, struct encoded_instruction *result) {

    // most instructions need mod/rm byte
    if (info->needs_modregrm || info->has_opcode_extension) {
        result->flags.have_modregrm = true;
        
        // first operand one
        if (instr->operand1.is_register)
        {
            // easy set Mod to "11" and "R/M" to the register
            result->modregrm_byte |= (0x3 << 6);
            result->modregrm_byte |= (instr->operand1.per_type.reg & 0x7);
        }
        else if (instr->operand1.is_memory_by_displacement)
        {
            // special case, set mod to '00' and r/m to '101'
            result->modregrm_byte |= (0x0 << 6);
            result->modregrm_byte |= (0x5);

            // fixed size 32-bits displacement in this case
            *(long *)result->displacement = instr->operand1.per_type.mem.displacement;
            result->displacement_bytes_count = 4;
        }
        else if (instr->operand1.is_memory_by_reg)
        {
            // see if we have an array notation (item must be 1,2,4 or 8)
            if (instr->operand1.per_type.mem.array_item_size == 0)
            {
                // it's invalid to use SP as pointer, the notation is used for the SIB
                if (instr->operand1.per_type.mem.pointer_reg == REG_SP)
                    return false;
                
                // no SIB byte, proceed normally
                // "Mod" part will be set by the displacement size
                result->modregrm_byte |= instr->operand1.per_type.mem.pointer_reg;
            }
            else // we have item size, use SIB
            {
                // flag SIB presence in Reg part of ModRegRM (32bits mode)
                result->modregrm_byte |= (0x4); // '100' signals SIB presense
                result->flags.have_sib = true;

                // scale x1, x2, x4, x8
                switch (instr->operand1.per_type.mem.array_item_size) {
                    case 1: result->sib_byte |= (0x0 << 6); break;
                    case 2: result->sib_byte |= (0x1 << 6); break;
                    case 4: result->sib_byte |= (0x2 << 6); break;
                    case 8: result->sib_byte |= (0x3 << 6); break;
                    default: return false;
                }
                // register to use for array index
                result->sib_byte |= ((instr->operand1.per_type.mem.array_index_reg & 0x7) << 3);
                // register to use for base
                result->sib_byte |= (instr->operand1.per_type.mem.pointer_reg & 0x7);
            }
            
            // SIB or not, add possible displacement
            if (instr->operand1.per_type.mem.displacement == 0) {
                // no displacement, set mod to 00
                result->modregrm_byte |= (0x0 << 6);
            } else if (instr->operand1.per_type.mem.displacement >= -128 && instr->operand1.per_type.mem.displacement <= 127) {
                // one byte signed displacement follows
                result->modregrm_byte |= (0x1 << 6);
                result->displacement[0] = (char)instr->operand1.per_type.mem.displacement;
                result->displacement_bytes_count = 1;
            } else {
                // full four bytes signed displacement follows
                result->modregrm_byte |= (0x2 << 6);
                *(long *)result->displacement = instr->operand1.per_type.mem.displacement;
                result->displacement_bytes_count = 4;
            }
        } 

        // then, operand two (if it's immediate, see other function)
        if (info->has_opcode_extension) {
            result->modregrm_byte |= ((info->opcode_extension_value & 0x7) << 3);
        }
        else if (instr->operand2.is_register) {
            result->modregrm_byte |= ((instr->operand2.per_type.reg & 0x7) << 3);
        }
    }

    // some instructions take a displacement without modregrm byte (e.g. JMP)
    if (instr->operand1.is_memory_by_displacement && info->displacement_without_modrm && !info->needs_modregrm) {
        // must save relocation position!!!!!!
        *(long *)result->displacement = instr->operand1.per_type.mem.displacement;
        result->displacement_bytes_count = 4;
        return true;
    }
    
    return true;
}

static bool encode_asm_instr_immediate(asm_instruction *instr, struct encoding_info *info, struct encoded_instruction *result) {

    // maybe we don't need anything
    if (!instr->operand2.is_immediate)
        return true;
    
    // verify the instruction chosen supports immediate values
    if (!info->supports_immediate_value)
        return false;
    
    // set the full immediate value
    *(long *)result->immediate = (long)instr->operand2.per_type.immediate;
    result->immediate_bytes_count = 4;

    // see if we can shorten the bytes added
    // "sign_expanded_immediate" takes the place of direction
    // 0=immediate as indicated by size bit (8/32 bits),
    // 1=immediate is 1-byte signed number, to be sign extended
    if (info->has_sign_expanded_immediate_bit 
        && instr->operand2.per_type.immediate >= -128
        && instr->operand2.per_type.immediate <= 127) {
            // set the sign expand bit and add just one immediate byte
            result->opcode_byte |= 0x2;
            *(char *)result->immediate = (char)instr->operand2.per_type.immediate;
            result->immediate_bytes_count = 1;
    }

    return true;
}

bool encode_asm_instruction(asm_instruction *instr, struct encoding_info *info, struct encoded_instruction *result) {
    // logic in this function based largely on this page;
    // http://www.c-jump.com/CIS77/CPU/x86/lecture.html

    // maybe good example will be:
    // "MOV [BP+CX*4+16], -16", it contains 16bit prefix, ModRegRM, SIB, displacement, immediate.
    // it should become:  C7  44  8D   10    F0 FF FF FF
    //                    op  rm  sid  offs   immediate

    memset(result, 0, sizeof(struct encoded_instruction));
    if (!encode_asm_instr_opcode(instr, info, result))
        return false;
    if (!encode_asm_instr_operands(instr, info, result))
        return false;
    if (!encode_asm_instr_immediate(instr, info, result))
        return false;
    return true;
}
