#include "asm_instruction.h"

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

struct asm_operation {
    int operation; // ADD, SUB, etc. no sign cognizance
    int operands_size; // 8,16,32,64 (width bit for 8bits, 0x66 prefix for 16bits)
    bool direction_op1_to_op2; // if false, the opposite

    struct operand1 { // goes to the Mod+R/M part of the ModRegRM byte
        bool is_memory;
        bool is_register;
        union {
            enum gp_reg_family reg_family;
            struct {
                enum gp_reg_family pointer_reg;
                long displacement;   // 0 means no displacement
                enum gp_reg_family array_index_reg;
                int array_item_size; // must be 1,2,4 or 8 for SID to be used
            } memory_address;
        } per_type;
    } operand1; 

    struct operand2 {  // goes to the Reg part of the Mod+Reg+RM byte
        bool is_constant;
        bool is_register;
        union {
            long constant;
            enum gp_reg_family reg_family;
        } per_type;
    } operand2; 
};

void print_cpu_operation(struct asm_operation *oper, FILE *stream);

//---------------------------------------------------

struct encoded_instruction {
    struct {
        unsigned long have_instruction_prefix : 1;
        unsigned long have_address_size_prefix : 1;
        unsigned long have_operand_size_prefix : 1;
        unsigned long have_segment_override_prefix : 1;
        unsigned long have_opcode_expansion_byte: 1;
        unsigned long have_second_opcode_byte: 1;
        unsigned long have_modregrm: 1;
        unsigned long have_sib: 1;
    } flags;
    unsigned char instruction_prefix;
    unsigned char address_size_prefix;
    unsigned char operand_size_prefix;
    unsigned char segment_override_prefix;
    unsigned char opcode_expansion_byte;
    unsigned char opcode_byte; // always there
    unsigned char modregrm_byte;
    unsigned char sib_byte;
    unsigned char displacement[4];
    unsigned char displacement_bytes_count; // 0..4
    unsigned char immediate[4];
    unsigned char immediate_bytes_count;    // 0..4
};

void pack_encoded_instruction(struct encoded_instruction *i, char *buffer, int *buffer_length);
void print_encoded_instruction(struct encoded_instruction *i, FILE *stream);

// ---------------------

struct encoding_info {
    bool has_instruction_expansion_byte; // only a few instructions do
    unsigned char instruction_expansion_byte; // usually 0x0F;

    unsigned char base_opcode_byte; // the basis, adding direction and size to this
    bool has_width_bit;     // usually bit 0
    bool has_direction_bit; // usually bit 1
    char opcode_subcode; // valid values 0-7, -1 means N/A, used often with immediates
    bool is_reg_part_of_opcode;  // e.g. PUSH EBP (0x55)

    bool supports_immediate_value; // does in support immediate following?
    bool needs_modregrm; // means ModRegRM (and possibly SIB) byte is needed

    // usually bit 1 (not zero) of opcode, instead of direction bit
    // means that immediate can be shortened to 1 byte, instead of four
    bool has_sign_expanded_immediate_bit; 
};

bool encode_cpu_operation(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result);

// --------------------------------------------------------------
// implementation below here
// --------------------------------------------------------------

static bool encode_prefix_bytes(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result);
static bool encode_opcode_bytes(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result);
static bool encode_modregrm_sib_bytes(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result);
static bool encode_immediate_and_displacement_bytes(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result);

bool encode_cpu_operation(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result) {
    // logic in this function based largely on this page;
    // http://www.c-jump.com/CIS77/CPU/x86/lecture.html

    memset(result, 0, sizeof(struct encoded_instruction));
    if (!encode_prefix_bytes(oper, info, result))
        return false;
    if (!encode_opcode_bytes(oper, info, result))
        return false;
    if (!encode_modregrm_sib_bytes(oper, info, result))
        return false;
    if (!encode_immediate_and_displacement_bytes(oper, info, result))
        return false;
    return true;





    if (oper->operands_size == 16) {
        // if we operated in 16 bits and wanted 32 bits, we'd emit 0x67
        result->flags.have_operand_size_prefix = 1;
        result->operand_size_prefix = 0x66;
    }

    // find if instruction requires expansion byte and set it up.
    if (info->has_instruction_expansion_byte) {
        result->flags.have_opcode_expansion_byte = 1;
        result->opcode_expansion_byte = info->instruction_expansion_byte;
    }

    // verify the instruction chosen supports immediate values
    if (oper->operand2.is_constant && !info->supports_immediate_value) { 
        return false;
    }

    // this always happens
    result->opcode_byte = info->base_opcode_byte;

    if (oper->operand2.is_constant) {
        // REG part of ModRegRM is usually an opcode extension
        // ModRM part of ModRegRM as normal for the target of the action
        *(long *)result->immediate = (long)oper->operand2.per_type.constant;

        // direction bit becomes 0=constant as indicated by size bit (8/32 bits),
        //                       1=constant is 1-byte signed number, to be sign extended
        // see if we can shorten the bytes added
        if (info->has_sign_expanded_immediate_bit 
            && oper->operand2 >= -128
            && oper->operand2 <= 127) {
                // set the sign expand bit and add just one immediate byte
                result->opcode_byte |= 0x2;
                result->immediate_bytes_count = 1;
                *(char *)result->immediate = (char)oper->operand2.per_type.constant;
        }
    } else {
        if (info->has_direction_bit) {
            if (oper->direction_regmem_to_regconst)  // one means from R/M --> Reg in the ModRegR/M byte                
                result->opcode_byte |= 0x2;
            else // zero means from Reg --> R/M in the ModRegR/M byte
                result->opcode_byte &= ~0x2;
        }
    }

    // width bit works both for immediate and for reg-mem types of instructions
    if (info->has_width_bit) {
        if (oper->operands_size == 8) // zero indicates single byte operands
            result->opcode_byte &= ~0x1;
        else // one indicates full size operands
            result->opcode_byte |= 0x1;
    }

    // encode ModRegR/M byte as needed
    if (...) {
        // encode SIB byte as needed
        if (...) {

        }
    }
    
    // how about displacement?
}

static void encode_prefix_bytes(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result) {

}
static void encode_opcode_bytes(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result) {
    
}
static void encode_modregrm_sib_bytes(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result) {
    // ...
}
static void encode_immediate_and_displacement_bytes(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result) {
    
}

void pack_encoded_instruction(struct encoded_instruction *i, char *buffer, int *buffer_length) {
    if (i->flags.have_instruction_prefix) {
        *buffer++ = i->instruction_prefix;
        (*buffer_length)++;
    }
    ...
}

void print_cpu_operation(struct asm_operation *oper, FILE *stream) {
/*
    Mnemonic  Size   Operand1                  Dir    Operand2
    XXXXXXX   dword  [EAX+ECX*8+0x12345678]    <--    EDX / 0x12345678
*/
}

void print_encoded_instruction(struct encoded_instruction *i, FILE *stream) {
/*
    4 prefix bytes, 2 opcode bytes, modregrm, sib, 0-4 displacement, 0-4 immediate

    Ins Adr Opr Seg   Opc         ModR/N       SIB
    Pfx Siz Siz Ovr   Pfx Opc   Md Reg R/M  SS Idx Bse   Displacemnt   Immediate
     00  00  00  00    00  00   00.000.000  00.000.000   00 00 00 00   00 00 00 00 
     --  --  --  --    --  --   --.---.---  --.---.---   -- -- -- --   -- -- -- --
*/
}
