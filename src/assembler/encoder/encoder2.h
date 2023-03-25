#include "asm_instruction.h"

/*
    Based on http://aturing.umcs.maine.edu/~meadow/courses/cos335/Asm07-MachineLanguage.pdf
    And on http://www.c-jump.com/CIS77/CPU/x86/lecture.html

    Best reference is the "Intel's Software Developer's Manual", Volume 2, Appendix B2, table B-13
    Page 2691 on the Huge PDF that has all the volumes.
*/


enum instruction_operand {
    
    OD_REG1, // contents of a register
    OD_REG2,
    OD_REG,
    OD_MEM,   // to/from memory by [reg + displacement] or [direct address]
    OD_AX,    // the AL, AX, EAX, RAX family of registers
    OD_CL,    // the CL register
    OD_ONE,   // as in shift-by-one, or inc-by-one
    OD_IMM8,  // 8-bits immediate value
    OD_IMMf,  // full size immediate value (16/32/64 bits)
    OD_IMMw,  // immediate value depending on wide_flag (8 or 16/32/64 bits)
    OD_DIS,   // displacement, i.e. signed distance to IP (i think)
    OD_DISF,  // displacement full, absolute mem address
    OD____,
};

struct instruction_endoding_info {
    enum opcode op;
    enum instruction_operand op1_type;
    enum instruction_operand op2_type;
    char *encoding;
};


// it's so difficult, from 16 to 32 to 64 bits, there's all sorts of differences
// also, it seems this offers open options, for human programmers,
// not a uniform unterface for machines to take advantage of.
// differences on whether we are using wide/narrow operands 
// and differnces on using signed/not signed numbers.
// maybe that's why C has the "int" type so vague, 
// because it defaults to whatever the registers support.



struct instruction_endoding_info inst32_encoding_info[] =  {
// instruction, from,  to/by,   encoding
    OC_ADD,  OD_REG1, OD_REG2,  "0000 000w : 11 reg1 reg2",
    OC_ADD,  OD_REG2, OD_REG1,  "0000 001w : 11 reg1 reg2",
    OC_ADD,  OD_MEM,  OD_REG,   "0000 001w : mod reg r/m",
    OC_ADD,  OD_REG,  OD_MEM,   "0000 000w : mod reg r/m",
    OC_ADD,  OD_IMMw,  OD_REG,  "1000 00sw : 11 000 reg : immediate data",
    OC_ADD,  OD_IMMw,  OD_AX,   "0000 010w : immediate data",
    OC_ADD,  OD_IMMw,  OD_MEM,  "1000 00sw : mod 000 r/m : immediate data",
    OC_AND,  OD_REG1, OD_REG2,  "0010 000w : 11 reg1 reg2",
    OC_AND,  OD_REG2, OD_REG1,  "0010 001w : 11 reg1 reg2",
    OC_AND,  OD_MEM,  OD_REG,   "0010 001w : mod reg r/m",
    OC_AND,  OD_REG,  OD_MEM,   "0010 000w : mod reg r/m",
    OC_AND,  OD_IMMw,  OD_REG,  "1000 00sw : 11 100 reg : immediate data",
    OC_AND,  OD_IMMw,  OD_AX,   "0010 010w : immediate data",
    OC_AND,  OD_IMMw,  OD_MEM,  "1000 00sw : mod 100 r/m : immediate data",
    OC_CALL, OD_IMMf,  OD____,  "1110 1000 : full displacement", // direct
    OC_CALL, OD_REG,  OD____,   "1111 1111 : 11 010 reg", // indirect 
    OC_CALL, OD_MEM,  OD____,   "1111 1111 : mod 010 r/m", // indirect
    OC_CMP,  OD_REG1, OD_REG2,  "0011 100w : 11 reg1 reg2",
    OC_CMP,  OD_REG2, OD_REG1,  "0011 101w : 11 reg1 reg2",
    OC_CMP,  OD_MEM,  OD_REG,   "0011 100w : mod reg r/m",
    OC_CMP,  OD_REG,  OD_MEM,   "0011 101w : mod reg r/m",
    OC_CMP,  OD_IMMw,  OD_REG,  "1000 00sw : 11 111 reg : immediate data",
    OC_CMP,  OD_IMMw,  OD_AX,   "0011 110w : immediate data",
    OC_CMP,  OD_IMMw,  OD_MEM,  "1000 00sw : mod 111 r/m : immediate data",
    OC_DEC,  OD_REG,  OD____,   "1111 111w : 11 001 reg",         // basic encoding
    OC_DEC,  OD_REG,  OD____,   "0100 1 reg", // alternate encoding
    OC_DEC,  OD_MEM,  OD____,   "1111 111w : mod 001 r/m",
    OC_DIV,  OD_AX,   OD_REG,   "1111 011w : 11 110 reg",
    OC_DIV,  OD_AX,   OD_MEM,   "1111 011w : mod 110 r/m",
    OC_INC,  OD_REG,  OD____,   "1111 111w : 11 000 reg", // basic encoding
    OC_INC,  OD_REG,  OD____,   "0100 0 reg", // alternate encoding
    OC_INC,  OD_MEM,  OD____,   "1111 111w : mod 000 r/m",
    OC_INT,  OD_IMM8,  OD____,  "1100 1101 : imm", // immediate 8 bits
    OC_JMP,  OD_IMM8,  OD____,  "1110 1011 : 8-bit displacement", // short
    OC_JMP,  OD_IMMf,  OD____,  "1110 1001 : full displacement",  // direct
    OC_JMP,  OD_REG,  OD____,   "1111 1111 : 11 100 reg",  // indirect
    OC_JMP,  OD_MEM,  OD____,   "1111 1111 : mod 100 r/m", // indirect
    OC_JEQ,  OD_IMM8,  OD____,  "0111 0100 : 8-bit displacement", // 8-bit displacement
    OC_JNE,  OD_IMM8,  OD____,  "0111 0101 : 8-bit displacement", // 8-bit displacement
    OC_JLT,  OD_IMM8,  OD____,  "0111 0010 : 8-bit displacement", // 8-bit displacement
    OC_JLE,  OD_IMM8,  OD____,  "0111 0110 : 8-bit displacement", // 8-bit displacement
    OC_JGT,  OD_IMM8,  OD____,  "0111 0111 : 8-bit displacement", // 8-bit displacement
    OC_JGE,  OD_IMM8,  OD____,  "0111 0011 : 8-bit displacement", // 8-bit displacement
    OC_JEQ,  OD_IMMf,  OD____,  "0000 1111 : 1000 0100 : full displacement", // full displacement
    OC_JNE,  OD_IMMf,  OD____,  "0000 1111 : 1000 0101 : full displacement", // full displacement
    OC_JLT,  OD_IMMf,  OD____,  "0000 1111 : 1000 0010 : full displacement", // full displacement
    OC_JLE,  OD_IMMf,  OD____,  "0000 1111 : 1000 0110 : full displacement", // full displacement
    OC_JGT,  OD_IMMf,  OD____,  "0000 1111 : 1000 0111 : full displacement", // full displacement
    OC_JGE,  OD_IMMf,  OD____,  "0000 1111 : 1000 0011 : full displacement", // full displacement
    OC_LEA,  OD_MEM,  OD____,   "1000 1101 : mod reg r/m",
    OC_MOV,  OD_REG1, OD_REG2,  "1000 100w : 11 reg1 reg2",
    OC_MOV,  OD_REG2, OD_REG1,  "1000 101w : 11 reg1 reg2",
    OC_MOV,  OD_MEM,  OD_REG,   "1000 101w : mod reg r/m",
    OC_MOV,  OD_REG,  OD_MEM,   "1000 100w : mod reg r/m",
    OC_MOV,  OD_IMMw,  OD_REG,  "1100 011w : 11 000 reg : immediate data",
    OC_MOV,  OD_IMMw,  OD_REG,  "1011 w reg : immediate data", // alternate encoding
    OC_MOV,  OD_IMMw,  OD_MEM,  "1100 011w : mod 000 r/m : immediate data",
    OC_MOV,  OD_MEM,  OD_AX,    "1010 000w : full displacement",
    OC_MOV,  OD_AX,   OD_MEM,   "1010 001w : full displacement",
    OC_MUL,  OD_AX,   OD_REG,   "1111 011w : 11 100 reg",
    OC_MUL,  OD_AX,   OD_MEM,   "1111 011w : mod 100 r/m",
    OC_NEG,  OD_REG,  OD____,   "1111 011w : 11 011 reg",
    OC_NEG,  OD_MEM,  OD____,   "1111 011w : mod 011 r/m",
    OC_NOP,  OD____,  OD____,   "1001 0000",
    OC_NOT,  OD_REG,  OD____,   "1111 011w : 11 010 reg",
    OC_NOT,  OD_MEM,  OD____,   "1111 011w : mod 010 r/m",
    OC_OR,   OD_REG1, OD_REG2,  "0000 100w : 11 reg1 reg2",
    OC_OR,   OD_REG2, OD_REG1,  "0000 101w : 11 reg1 reg2",
    OC_OR,   OD_MEM,  OD_REG,   "0000 101w : mod reg r/m",
    OC_OR,   OD_REG,  OD_MEM,   "0000 100w : mod reg r/m",
    OC_OR,   OD_IMMw,  OD_REG,  "1000 00sw : 11 001 reg : immediate data",
    OC_OR,   OD_IMMw,  OD_AX,   "0000 110w : immediate data",
    OC_OR,   OD_IMMw,  OD_MEM,  "1000 00sw : mod 001 r/m : immediate data",
    OC_POP,  OD_REG,  OD____,   "1000 1111 : 11 000 reg",
    OC_POP,  OD_REG,  OD____,   "0101 1 reg",  // alternate encoding
    OC_POP,  OD_MEM,  OD____,   "1000 1111 : mod 000 r/m",
    OC_PUSH, OD_REG,  OD____,   "1111 1111 : 11 110 reg",
    OC_PUSH, OD_REG,  OD____,   "0101 0 reg", // alternate encoding
    OC_PUSH, OD_MEM,  OD____,   "1111 1111 : mod 110 r/m",
    OC_PUSH, OD_IMM8, OD____,   "0110 1010 : imm8", // 6a=imm8, 68=imm32
    OC_PUSH, OD_IMMf, OD____,   "0110 1000 : immf", // 6a=imm8, 68=imm32
    OC_RET,  OD____,  OD____,   "1100 0011", // without argument
    OC_RET,  OD_IMM,  OD____,   "1100 0010 : imm16", // adding 16bits imm to SP
    OC_SHL,  OD_REG,  OD_ONE,   "1101 000w : 11 100 reg", // shift by one
    OC_SHL,  OD_MEM,  OD_ONE,   "1101 000w : mod 100 r/m", // shift by one
    OC_SHL,  OD_REG,  OD_CL,    "1101 001w : 11 100 reg",
    OC_SHL,  OD_MEM,  OD_CL,    "1101 001w : mod 100 r/m",
    OC_SHL,  OD_REG,  OD_IMM8,  "1100 000w : 11 100 reg : imm8", // imm = num of bits
    OC_SHL,  OD_MEM,  OD_IMM8,  "1100 000w : mod 100 r/m : imm8", // imm = num of bits
    OC_SHR,  OD_REG,  OD_ONE,   "1101 000w : 11 101 reg",
    OC_SHR,  OD_MEM,  OD_ONE,   "1101 000w : mod 101 r/m",
    OC_SHR,  OD_REG,  OD_CL,    "1101 001w : 11 101 reg",
    OC_SHR,  OD_MEM,  OD_CL,    "1101 001w : mod 101 r/m",
    OC_SHR,  OD_REG,  OD_IMM8,  "1100 000w : 11 101 reg : imm8", // imm = num of bits
    OC_SHR,  OD_MEM,  OD_IMM8,  "1100 000w : mod 101 r/m : imm8", // imm = num of bits
    OC_SUB,  OD_REG1, OD_REG2,  "0010 100w : 11 reg1 reg2",
    OC_SUB,  OD_REG2, OD_REG1,  "0010 101w : 11 reg1 reg2",
    OC_SUB,  OD_MEM,  OD_REG,   "0010 101w : mod reg r/m",
    OC_SUB,  OD_REG,  OD_MEM,   "0010 100w : mod reg r/m",
    OC_SUB,  OD_IMM8, OD_REG,   "1000 00sw : 11 101 reg : immediate data",
    OC_SUB,  OD_IMM8, OD_AX,    "0010 110w : immediate data",
    OC_SUB,  OD_IMM8, OD_MEM,   "1000 00sw : mod 101 r/m : immediate data",
    OC_XOR,  OD_REG1, OD_REG2,  "0011 000w : 11 reg1 reg2",
    OC_XOR,  OD_REG2, OD_REG1,  "0011 001w : 11 reg1 reg2",
    OC_XOR,  OD_MEM,  OD_REG,   "0011 001w : mod reg r/m",
    OC_XOR,  OD_REG,  OD_MEM,   "0011 000w : mod reg r/m",
    OC_XOR,  OD_IMMw,  OD_REG,  "1000 00sw : 11 110 reg : immediate data",
    OC_XOR,  OD_IMMw,  OD_AX,   "0011 010w : immediate data",
    OC_XOR,  OD_IMMw,  OD_MEM,  "1000 00sw : mod 110 r/m : immediate data",
};

int find_encoding(enum opcode op, enum instruction_operand od1, enum instruction_operand od2) {
    for (int i = 0; i < sizeof(inst32_encoding_info)/sizeof(inst32_encoding_info[0]); i++) {
        if (inst32_encoding_info[i].op = op
            && inst32_encoding_info[i].op1_type == od1
            && inst32_encoding_info[i].op2_type == od2)
            return i;
    }
    return -1;
}

bool encode(
        int reg1, int reg2,   // AX=0, CX, DX, BX, SP, BP, SI, DI=7, 3 bits each
        int rm,  // [BX+SI]=0, [BX+DI], [BP+SI], [BP+DI], [SI], [DI], [BP], [BX]=7
        int mod, // 00 = no offset, 01 = 8-bits offset, 10 = 16-bits offset
        bool wide_flag,       // 0 = use 8bit register, 1 = use 16/32/64 bit register depending on mode
        bool sign_extend,     // 1 = sign-extend to fill 16/32 bits destination. 0 = don't (see ADD)
        int immediate,
        int displacement,
        int bits_mode, // 8, 16, 32, or 64
        char encoding_string, // from the table above
        char *output_buffer,  // output buffer, at least 15 bytes long
        int *output_size      // how many bytes were generated
) {
    char *enc = encoding_string;
    unsigned char curr_byte = 0;
    int curr_bits = 0;
    *output_size = 0;

    while (*enc != '\0') {
        while (*enc == ' ') { enc++; } // ignore whitespace
        if (*enc == '\0') break;


        if (*enc == '0') {  // put 1 bit, value 0
            curr_byte = (curr_byte << 1) | 0x0;
            curr_bits++;
            enc++;
        } else if (*enc == '1') { // put 1 bit, value 1
            curr_byte = (curr_byte << 1) | 0x1;
            curr_bits++;
            enc++;
        } else if (*enc == 'w') {
            // put 1 bit, value 1 for full size (16/32/64 bits) or 0 for single byte operation
            curr_byte = (curr_byte << 1) | (wide_flag & 0x1);
            curr_bits++;
            enc++;
        } else if (*enc == 's') {
            // put 1 bit, value 1 for full size (16/32/64 bits) or 0 for single byte operation
            curr_byte = (curr_byte << 1) | (sign_extend & 0x1);
            curr_bits++;
            enc++;
        } else if (memcmp(enc, "reg", 3) == 0) {
            // we need to put 3 bits, reg1 or reg (e.g. 000=AX, 001=CX etc)
            curr_byte = (curr_byte << 3) | (reg1 & 0x7);
            curr_bits += 3;
            enc += 3;
        } else if (memcmp(enc, "reg1", 4) == 0) {
            // we need to put 4 bits, reg1 or reg (e.g. 000=AX, 001=CX etc)
            curr_byte = (curr_byte << 3) | (reg1 & 0x7);
            curr_bits += 3;
            enc += 4;
        } else if (memcmp(enc, "reg2", 4) == 0) {
            // we need to put 4 bits, reg1 or reg (e.g. 000=AX, 001=CX etc)
            curr_byte = (curr_byte << 3) | (reg2 & 0x7);
            curr_bits += 3;
            enc += 4;
        } else if (memcmp(enc, "mod", 3) == 0) {
            // we need to put 2 bits, memory access mode (e.g. with 0, 8, 16 bits disposition)
            curr_byte = (curr_byte << 2) | (mod & 0x3);
            curr_bits += 2;
            enc += 3;
        } else if (memcmp(enc, "r/m", 3) == 0) {
            // we need to put 3 bits, memory access pointer selector
            // i.e. 000=[BX+SI], [BP], [SI], [DI]
            curr_byte = (curr_byte << 3) | (rm & 0x7);
            curr_bits += 3;
            enc += 3;
        } else if (memcmp(enc, "imm8", 4) == 0) {
            // we need to put 8-bits immediate
            if ((immediate & 0xFF) != immediate) {
                error(NULL, 0, "immediate value would be truncated")
                return false;
            }
            output_buffer[*output_size] = (immediate & 0xFF);
            (*output_size)++;
            enc += 4;
        } else if (memcmp(enc, "imm16", 5) == 0) {
            // we need to put 16-bits immediate
            if ((immediate & 0xFFFF) != immediate) {
                error(NULL, 0, "immediate value would be truncated")
                return false;
            }
            output_buffer[*output_size] = (immediate & 0xFFFF);
            (*output_size)++;
            enc += 5;
        } else if (memcmp(enc, "imm16111111111111111111111", 5) == 0) {
            // we need to put 16-bits immediate
            if ((immediate & 0xFFFF) != immediate) {
                error(NULL, 0, "immediate value would be truncated")
                return false;
            }
            output_buffer[*output_size] = (immediate & 0xFFFF);
            (*output_size)++;
            enc += 5;
        } else if (memcmp(enc, "immw", 4) == 0) {
            // we need to put immediate (which size???)
            enc += 4;
        } else if (*enc == ':') {
            // push this byte out to buffer
            output_buffer[*output_size] = curr_byte;
            (*output_size)++;
            curr_byte = 0;
            curr_bits++;
            enc++;
        } else {
            error(NULL, 0, "unknown encoding... '%s'", enc);
        }
    }

    // push the final byte out to the buffer
    if (curr_bits > 0) {
        output_buffer[*output_size] = curr_byte;
        (*output_size)++;
    }
    return true;
}

