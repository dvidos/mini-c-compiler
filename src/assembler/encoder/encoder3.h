#include 

enum gp_reg_family {  // bits 3,4,5 in ModRM byte
    RF_AX = 0,
    RF_CX = 1,
    RF_DX = 2,
    RF_BX = 3, 
    RF_SP = 4, // if mod is not '11' (0x3) a SIB follows
    RF_BP = 5,
    RF_SI = 6,
    RF_DI = 7
}; 
enum one_byte_register { // if operand.size_flag is 0
    R1B_AL = 0,
    R1B_CL = 1,
    R1B_DL = 2,
    R1B_BL = 3,
    R1B_AH = 4, // if mod is 0,1 or 2, a SIB follows
    R1B_CH = 5,
    R1B_DH = 6,
    R1B_BH = 7
};
enum two_bytes_register { // requires special prefix
    R2B_AX = 0,
    R2B_CX = 1,
    R2B_DX = 2,
    R2B_BX = 3,
    R2B_SP = 4, // if mod is 0,1 or 2, a SIB follows
    R2B_BP = 5,
    R2B_SI = 6,
    R2B_DI = 7
};
enum four_bytes_register { // if operand.size_flag is 1
    R4B_EAX = 0,
    R4B_ECX = 1,
    R4B_EDX = 2,
    R4B_EBX = 3,
    R4B_ESP = 4, // if mod is 0,1 or 2, a SIB follows
    R4B_EBP = 5,
    R4B_ESI = 6,
    R4B_EDI = 7
};
enum mem_addressing_mode {
    // r/m will contain the register that holds the address
    // note that for MEM_ADDR and AH/SP/ESP, a SIB is expected
    MODE_MEM_ADDR_WITHOUT_DISPLACEMENT = 0,
    MODE_MEM_ADDR_DISPLACEMENT_1BYTE   = 1,
    MODE_MEM_ADDR_DISPLACEMENT_4BYTES  = 2,
    // register size depends on operand.size_flag and possible 16-bit prefix
    MODE_REG_CONTENTS                  = 3, 
};
enum sib_scale {
    SIB_SCALE_1 = 0,
    SIB_SCALE_2 = 1, // e.g. arrays of words
    SIB_SCALE_4 = 2, // e.g. arrays of dwords
    SIB_SCALE_8 = 4  
};
enum sib_index {
    SIB_IDX_EAX = 0,
    SIB_IDX_ECX = 1,
    SIB_IDX_EDX = 2,
    SIB_IDX_EBX = 3,
    SIB_IDX_ILLEGAL = 4, // unsupported
    SIB_IDX_EBP = 5,
    SIB_IDX_ESI = 6,
    SIB_IDX_EDI = 7,
};
enum sib_base {
    SIB_BASE_EAX = 0,
    SIB_BASE_ECX = 1,
    SIB_BASE_EDX = 2,
    SIB_BASE_EBX = 3,
    SIB_BASE_ESP = 4,
    SIB_BASE_SPECIAL = 5, // mod=0: no base, mod=1,2: EBP for base
    SIB_BASE_ESI = 6,
    SIB_BASE_EDI = 7,
};

struct blown_up_instruction {
    struct operand {
        // 6 bits usually, e.g. 000000 = ADD
        unsigned char opcode;  

        // if true, the action is MODR/M towards REG
        // if false, it's from REG towards MODR/M
        bool modrm_to_reg_direction; 

        // if true, operands are 32 bits, if false, 8 bits.
        // for 16 bits (or 64?) operands, special prefix is required
        bool full_size_operands;   
    } operand;

    //   7   6   5   4   3   2   1   0  
    // +---+---+---+---+---+---+---+---+
    // |  MOD  |    REG    |    R/M    |
    // +---+---+---+---+---+---+---+---+
    struct mod_reg_rm_byte {
        enum gp_reg_family reg;  // always points to a register (or opcode extension)
        enum mem_addressing_mode mod;
        union {
            // size of these registers defined by operand.size_flag and prefix for 16 bits
            enum one_byte_register   one_byte_register;   // if operand.size_flag is 0
            enum two_bytes_register  two_bytes_register;  // requires special prefix
            enum four_bytes_register four_bytes_register; // if operand.size_flag is 1
            // note that for MODE_MEM_ADDR_xxx and REG_SP, a SIB is expected
        } rm;
    } mod_reg_rm_byte;

    //   7   6   5   4   3   2   1   0  
    // +---+---+---+---+---+---+---+---+
    // | Scale |   Index   |   Base    |
    // +---+---+---+---+---+---+---+---+
    struct scaled_index_byte {
        enum sib_base base;  // base address of array
        enum sib_index index; // index of item
        enum sib_scale scale;     // size of element (1,2,4,8 bytes)
    } scaled_index_byte;
}

