#include <stdlib.h>
#include <stdint.h>
#include "encoder.h"


static bool x86_encoder_encode(struct x86_encoder *encoder, struct instruction *instr, struct bin_buffer *target);

struct x86_encoder *new_x86_encoder(enum x86_cpu_mode mode) {
    struct x86_encoder *encoder = malloc(sizeof(struct x86_encoder));
    encoder->mode = mode;

    encoder->encode = x86_encoder_encode;
};

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
// how shall we encode symbol references????
// encoding should allow for referencing symbols, not only final binary code
// we would need a table, that indicates how each instruction is encoded,
// i.e. whether it needs a +r8, or a ModR/M or Immediate etc
// then encoding would be really simple, just branching with
// the different ways to encode instructions.

static bool x86_encoder_encode(struct x86_encoder *encoder, struct instruction *instr, struct bin_buffer *target) {
    if (encoder->mode != MODE_PROTECTED) {
        return false;
    }
    // then find the symbol encoding information and perform the binary bit crunching
    return false;
}
