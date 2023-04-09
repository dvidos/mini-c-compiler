```c

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
/*
    essentially, since the instruction encoding is so complex
    we need to find a simpler way to approach it.
    all the documentation is useful for disassembling, but does
    not help how to go about choosing how to assemble something.
    
    Displacements could/should be used for structure members.
    SIBs could/should be used for arrays of 1/2/4/8 bytes elements

    But all the above complexity can go away, by code that 
    calculates the exact memory address and puts it in a register.

    For immediate constants, things are even more complex
    Operands have the most significant bit 1,
    Mod+RM have different meanings, direction bit is different, etc.

    Maybe we should start amazingly simple.
*/
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

// static inline u8 modrm_byte(int mode, int reg, int reg_mem);
// static inline u8 sib_byte(int scale, int index, int base);
// static bool encode_single_byte_instruction_adding_reg_no(struct x86_encoder *enc, u8 base_opcode, int reg_no);
// static bool encode_ext_instr_direct_reg(struct x86_encoder *enc, u8 opcode, u8 ext_opcode, int reg_no);
// static bool encode_ext_instr_mem_by_register(struct x86_encoder *enc, u8 opcode, u8 ext_opcode, int reg_no, int displacement);
// static bool encode_ext_instr_mem_by_symbol(struct x86_encoder *enc, u8 opcode, u8 ext_opcode, char *symbol_name);



```
