


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
