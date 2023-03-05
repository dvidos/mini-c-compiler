#include <stdlib.h>
#include <stdint.h>
#include "encoder.h"

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

// if we extend our "expr_target" structure, it could be used as operand for asm expressions.
// i.e. "MOV EDX, [EBX + 16*EAX + 4]" to do "y = arr[i].next"
// so maybe we could build things up instead of down.
// see what MOV takes as arguments: https://www.felixcloutier.com/x86/mov
// operands: temp register, actual register, memory location or symbol, offset, immediate
// operand data size may be: byte, word, dword, quad (8, 16, 32 or 64 bits)

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


static bool x86_encoder_encode(struct x86_encoder *enc, struct instruction *instr);
static void x86_encoder_reset(struct x86_encoder *enc);
static void x86_encoder_free(struct x86_encoder *enc);

struct x86_encoder *new_x86_encoder(enum x86_cpu_mode mode, buffer *code_out, reloc_list *relocations_out) {
    struct x86_encoder *enc = malloc(sizeof(struct x86_encoder));
    enc->mode = mode;

    enc->output = code_out;
    enc->relocations = relocations_out;

    enc->encode = x86_encoder_encode;
    enc->reset = x86_encoder_reset;
    enc->free = x86_encoder_free;
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



static bool x86_encoder_encode(struct x86_encoder *enc, struct instruction *instr) {
    if (enc->mode != CPU_MODE_PROTECTED) {
        return false;
    }

    switch (instr->opcode) {
        case OC_NOP:
            enc->output->add_byte(enc->output, 0x90); // simplest case
            break;

        case OC_PUSH:
            if (instr->op1.type == OT_REGISTER) {
                // this one cannot push segment registers, only general ones
                return encode_single_byte_instruction_adding_reg_no(enc, 0x50, instr->op1.value);

            } else if (instr->op1.type == OT_MEM_DWORD_POINTED_BY_REG) {
                // Intel gives this as: "FF /6   ModRM:r/m (r)"
                return encode_ext_instr_mem_by_register(enc, 0xFF, 6, instr->op1.value, instr->op1.offset);

            } else if (instr->op1.type == OT_SYMBOL_MEM_ADDRESS) {
                // Intel gives this as: "FF /6   ModRM:r/m (r)"
                return encode_ext_instr_mem_by_symbol(enc, 0xFF, 6, instr->op1.symbol_name);

            } else if (instr->op1.type == OT_IMMEDIATE) {
                // simple 68 + value
                if (instr->op1.value >= -128 && instr->op1.value <= 127) {
                    enc->output->add_byte(enc->output, 0x6a);
                    enc->output->add_byte(enc->output, (u8)instr->op1.value);
                } else {
                    enc->output->add_byte(enc->output, 0x68);
                    enc->output->add_dword(enc->output, instr->op1.value);
                }
                return true;
            } else {
                return false;
            }
            break;
        case OC_POP:
            if (instr->op1.type == OT_REGISTER) {
                // this one cannot push segment registers, only general ones
                return encode_single_byte_instruction_adding_reg_no(enc, 0x58, instr->op1.value);

            } else if (instr->op1.type == OT_MEM_DWORD_POINTED_BY_REG) {
                // Intel gives this as: "8F /0"
                return encode_ext_instr_mem_by_register(enc, 0x8F, 0, instr->op1.value, instr->op1.offset);

            } else if (instr->op1.type == OT_SYMBOL_MEM_ADDRESS) {
                // Intel gives this as: "8F /6   ModRM:r/m (r)"
                return encode_ext_instr_mem_by_symbol(enc, 0x8F, 0, instr->op1.symbol_name);

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
            if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_IMMEDIATE) {
                // B8+reg, imm32
                enc->output->add_byte(enc->output, 0xB8 + (instr->op1.value & 0x7));
                enc->output->add_dword(enc->output, instr->op2.value);
            } else if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_REGISTER) {
                // "89 /r"  (mode 11, reg=src, r/m=dest)
                enc->output->add_byte(enc->output, 0x89);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, instr->op2.value, instr->op1.value));
            } else if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_MEM_DWORD_POINTED_BY_REG) {
                // "8B /r"  (mode , reg=src, r/m=dest)
                return encode_ext_instr_mem_by_register(enc, 
                    0x8B, instr->op1.value, instr->op2.value, instr->op2.offset);
            } else if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_SYMBOL_MEM_ADDRESS) {
                encode_single_byte_instruction_adding_reg_no(enc, 0xB8, instr->op1.value);
                enc->relocations->add(enc->relocations, enc->output->length, instr->op2.symbol_name, RT_ABS_32);
                enc->output->add_dword(enc->output, 0xFFFFFFFF);
                return true;
                
            } else if (instr->op1.type == OT_MEM_DWORD_POINTED_BY_REG && instr->op2.type == OT_REGISTER) {

            } else if (instr->op1.type == OT_SYMBOL_MEM_ADDRESS && instr->op2.type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0xA3);
                enc->relocations->add(enc->relocations, enc->output->length, instr->op1.symbol_name, RT_ABS_32);
                enc->output->add_dword(enc->output, 0xFFFFFFFF);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 0, instr->op2.value));
                return true;

            } else if (instr->op1.type == OT_MEM_DWORD_POINTED_BY_REG && instr->op2.type == OT_IMMEDIATE) {
            } else if (instr->op1.type == OT_SYMBOL_MEM_ADDRESS && instr->op2.type == OT_IMMEDIATE) {
            } else {
                return false;
            }
            break;
        case OC_INT:
            // should be easy
            enc->output->add_byte(enc->output, 0xCD);
            enc->output->add_byte(enc->output, instr->op1.value);
            return true;
            break;

        case OC_RET:
            // near ret assumes no change in CS
            enc->output->add_byte(enc->output, 0xC3);
            return true;
            break;

        case OC_INC:
            if (instr->op1.type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x40 + (instr->op1.value & 0x7));
                return true;
            } else if (instr->op1.type == OT_MEM_DWORD_POINTED_BY_REG) {
                return encode_ext_instr_mem_by_register(enc, 0xFF, 0, instr->op1.value, instr->op1.offset);
            } else if (instr->op1.type == OT_SYMBOL_MEM_ADDRESS) {
                return encode_ext_instr_mem_by_symbol(enc, 0xFF, 0, instr->op1.symbol_name);
            }
            break;
        case OC_DEC:
            if (instr->op1.type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x48 + (instr->op1.value & 0x7));
                return true;
            } else if (instr->op1.type == OT_MEM_DWORD_POINTED_BY_REG) {
                return encode_ext_instr_mem_by_register(enc, 0xFF, 1, instr->op1.value, instr->op1.offset);
            } else if (instr->op1.type == OT_SYMBOL_MEM_ADDRESS) {
                return encode_ext_instr_mem_by_symbol(enc, 0xFF, 1, instr->op1.symbol_name);
            }
            break;
        case OC_NEG:
            if (instr->op1.type == OT_REGISTER) {
                // "F7/3"
                enc->output->add_byte(enc->output, 0xF7);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 3, instr->op1.value));
                return true;
            } else if (instr->op1.type == OT_MEM_DWORD_POINTED_BY_REG) {
                // "F7/3"
                return encode_ext_instr_mem_by_register(enc, 0xF7, 3, instr->op1.value, instr->op1.offset);
            } else if (instr->op1.type == OT_SYMBOL_MEM_ADDRESS) {
                // "F7/3"
                return encode_ext_instr_mem_by_symbol(enc, 0xF7, 3, instr->op1.symbol_name);
            }
            break;
        case OC_NOT:
            if (instr->op1.type == OT_REGISTER) {
                // "F7/2"
                enc->output->add_byte(enc->output, 0xF7);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 2, instr->op1.value));
                return true;
            } else if (instr->op1.type == OT_MEM_DWORD_POINTED_BY_REG) {
                // "F7/2"
                return encode_ext_instr_mem_by_register(enc, 0xF7, 2, instr->op1.value, instr->op1.offset);
            } else if (instr->op1.type == OT_SYMBOL_MEM_ADDRESS) {
                // "F7/2"
                return encode_ext_instr_mem_by_symbol(enc, 0xF7, 2, instr->op1.symbol_name);
            }
            break;
        case OC_CALL:
            if (instr->op1.type == OT_REGISTER) {
                // "FF/2"
                enc->output->add_byte(enc->output, 0xFF);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 2, instr->op1.value));
                return true;
            } else if (instr->op1.type == OT_MEM_DWORD_POINTED_BY_REG) {
                // "FF/2"
                return encode_ext_instr_mem_by_register(enc, 0xFF, 2, instr->op1.value, instr->op1.offset);
            } else if (instr->op1.type == OT_SYMBOL_MEM_ADDRESS) {
                // "FF/2", we'll use a CODE_SEG override to make this an absolute call,
                //         otherwise it will be relative to the next instruction
                //         and it'd be encoded as "e8 01 00 00 00" with dword as relative offset
                enc->output->add_byte(enc->output, 0x2E);
                return encode_ext_instr_mem_by_symbol(enc, 0xFF, 2, instr->op1.symbol_name);
            }
            break;
        case OC_ADD:
            if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x01);
                // for the "reg <- reg" cases, we put the destination in "reg" and source in "reg_mom"
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, instr->op2.value, instr->op1.value));
                return true;
            } else if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_IMMEDIATE) {
                enc->output->add_byte(enc->output, 0x81);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 0, instr->op1.value));
                enc->output->add_dword(enc->output, instr->op2.value);
                return true;
            }
            break;
        case OC_SUB:
            if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x29);
                // for the "reg <- reg" cases, we put the destination in "reg" and source in "reg_mom"
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, instr->op2.value, instr->op1.value));
                return true;
            } else if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_IMMEDIATE) {
                enc->output->add_byte(enc->output, 0x81);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 5, instr->op1.value));
                enc->output->add_dword(enc->output, instr->op2.value);
                return true;
            }
            break;
        case OC_IMUL:
            break;
        case OC_IDIV:
            break;
        case OC_AND:
            if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x21);
                // for the "reg <- reg" cases, we put the destination in "reg" and source in "reg_mom"
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, instr->op2.value, instr->op1.value));
                return true;
            }
            break;
        case OC_OR:
            if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x09);
                // for the "reg <- reg" cases, we put the destination in "reg" and source in "reg_mom"
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, instr->op2.value, instr->op1.value));
                return true;
            }
            break;
        case OC_XOR:
            if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_REGISTER) {
                enc->output->add_byte(enc->output, 0x31);
                // for the "reg <- reg" cases, we put the destination in "reg" and source in "reg_mom"
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, instr->op2.value, instr->op1.value));
                return true;
            }
            break;
        case OC_SHR:
            if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_IMMEDIATE) {
                // "C1/5 SHR r/m32, imm8"
                enc->output->add_byte(enc->output, 0xC1);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 5, instr->op1.value));
                enc->output->add_byte(enc->output, instr->op2.value);
                return true;
            }
            break;
        case OC_SHL:
            if (instr->op1.type == OT_REGISTER && instr->op2.type == OT_IMMEDIATE) {
                // "C1/4 SHL r/m32, imm8"
                enc->output->add_byte(enc->output, 0xC1);
                enc->output->add_byte(enc->output, modrm_byte(MODE_DIRECT_REGISTER, 4, instr->op1.value));
                enc->output->add_byte(enc->output, instr->op2.value);
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

static void x86_encoder_reset(struct x86_encoder *enc) {
    enc->output->clear(enc->output);
    enc->relocations->clear(enc->relocations);
}

static void x86_encoder_free(struct x86_encoder *enc) {
    enc->output->free(enc->output);
    enc->relocations->free(enc->relocations);
    free(enc);
}
