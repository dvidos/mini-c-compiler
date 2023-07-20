#include <string.h>
#include <stdio.h>
#include "encoded_instruction.h"


void pack_encoded_instruction(encoded_instruction *inst, bin *buff) {

    // prefixes
    if (inst->flags.have_instruction_prefix)
        bin_add_byte(buff, inst->values.instruction_prefix);

    if (inst->flags.have_address_size_prefix)
        bin_add_byte(buff, inst->values.address_size_prefix);
    
    if (inst->flags.have_operand_size_prefix)
        bin_add_byte(buff, inst->values.operand_size_prefix);

    if (inst->flags.have_segment_override_prefix)
        bin_add_byte(buff, inst->values.segment_override_prefix);

    if (inst->flags.have_rex_prefix)
        bin_add_byte(buff, inst->values.rex_prefix);

    // opcode(s)
    if (inst->flags.have_opcode_expansion_byte)
        bin_add_byte(buff, inst->values.opcode_expansion_byte);

    bin_add_byte(buff, inst->values.opcode_byte);

    // operands
    if (inst->flags.have_modregrm)
        bin_add_byte(buff, inst->values.modregrm_byte);

    if (inst->flags.have_sib)
        bin_add_byte(buff, inst->values.sib_byte);

    // displacement & immediate
    if (inst->values.displacement_bytes_count > 0)
        bin_add_mem(buff, inst->values.displacement, inst->values.displacement_bytes_count);

    if (inst->values.immediate_bytes_count > 0)
        bin_add_mem(buff, inst->values.immediate, inst->values.immediate_bytes_count);
}

void encoded_instruction_to_str(encoded_instruction *inst, str *s) {
/*
    4 prefix bytes, 2 opcode bytes, modregrm, sib, 0-4 displacement, 0-4 immediate

    "Ins Adr Opr Seg     Opc       ModR/M       SIB                                "
    "Pfx Siz Siz Ovr REX Pfx Opc Md Reg R/M  SS Idx Bse   Displacemnt   Immediate  "
    "00  --  00  --  --  00  00  00.000.000  00.000.000   00 00 00 00   00 00 -- --"
     012345678901234567890123456789012345678901234567890123456789012345678901234567890
     0         1         2         3         4         5         6         7         8
*/
    char num[16];
    char buffer[128];

    if (inst == NULL) {
        str_cats(s, "Ins Adr Opr Seg     Opc     --ModR/M-- ---SIB----                        \n");
        str_cats(s, "Pfx Siz Siz Ovr REX Pfx Opc Md Reg R/M SS Idx Bse Displacemnt Immediate  ");
        return;
    }
    //                 0         1         2         3         4         5         6         7         8
    //                 012345678901234567890123456789012345678901234567890123456789012345678901234567890
    strcpy(buffer,    "--  --  --  --  --  --  --  --.---.--- --.---.--- --.--.--.-- --.--.--.--");

    if (inst->flags.have_instruction_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->values.instruction_prefix);
        memcpy(buffer + 0, num, 2);
    }
    if (inst->flags.have_address_size_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->values.address_size_prefix);
        memcpy(buffer + 4, num, 2);
    }
    if (inst->flags.have_operand_size_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->values.operand_size_prefix);
        memcpy(buffer + 8, num, 2);
    }
    if (inst->flags.have_segment_override_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->values.segment_override_prefix);
        memcpy(buffer + 12, num, 2);
    }
    if (inst->flags.have_opcode_expansion_byte) {
        sprintf(num, "%02x", (unsigned char)inst->values.opcode_expansion_byte);
        memcpy(buffer + 16, num, 2);
    }
    if (inst->flags.have_rex_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->values.rex_prefix);
        memcpy(buffer + 16, num, 2);
    }
    sprintf(num, "%02x", (unsigned char)inst->values.opcode_byte);
    memcpy(buffer + 20, num, 2);

    if (inst->flags.have_modregrm) {
        sprintf(num, "%d%d.%d%d%d.%d%d%d", 
            (inst->values.modregrm_byte >> 7) & 0x1,
            (inst->values.modregrm_byte >> 6) & 0x1,
            (inst->values.modregrm_byte >> 5) & 0x1,
            (inst->values.modregrm_byte >> 4) & 0x1,
            (inst->values.modregrm_byte >> 3) & 0x1,
            (inst->values.modregrm_byte >> 2) & 0x1,
            (inst->values.modregrm_byte >> 1) & 0x1,
            (inst->values.modregrm_byte >> 0) & 0x1);
        memcpy(buffer + 24, num, 10);
    }
    if (inst->flags.have_sib) {
        sprintf(num, "%d%d.%d%d%d.%d%d%d", 
            (inst->values.sib_byte >> 7) & 0x1,
            (inst->values.sib_byte >> 6) & 0x1,
            (inst->values.sib_byte >> 5) & 0x1,
            (inst->values.sib_byte >> 4) & 0x1,
            (inst->values.sib_byte >> 3) & 0x1,
            (inst->values.sib_byte >> 2) & 0x1,
            (inst->values.sib_byte >> 1) & 0x1,
            (inst->values.sib_byte >> 0) & 0x1);
        memcpy(buffer + 35, num, 10);
    }
    if (inst->values.displacement_bytes_count > 0) {
        sprintf(num, "%02x", (unsigned char)inst->values.displacement[0]);
        memcpy(buffer + 46, num, 2);
    }
    if (inst->values.displacement_bytes_count > 1) {
        sprintf(num, "%02x", (unsigned char)inst->values.displacement[1]);
        memcpy(buffer + 49, num, 2);
    }
    if (inst->values.displacement_bytes_count > 2) {
        sprintf(num, "%02x", (unsigned char)inst->values.displacement[2]);
        memcpy(buffer + 52, num, 2);
    }
    if (inst->values.displacement_bytes_count > 3) {
        sprintf(num, "%02x", (unsigned char)inst->values.displacement[3]);
        memcpy(buffer + 55, num, 2);
    }
    if (inst->values.immediate_bytes_count > 0) {
        sprintf(num, "%02x", (unsigned char)inst->values.immediate[0]);
        memcpy(buffer + 58, num, 2);
    }
    if (inst->values.immediate_bytes_count > 1) {
        sprintf(num, "%02x", (unsigned char)inst->values.immediate[1]);
        memcpy(buffer + 61, num, 2);
    }
    if (inst->values.immediate_bytes_count > 2) {
        sprintf(num, "%02x", (unsigned char)inst->values.immediate[2]);
        memcpy(buffer + 64, num, 2);
    }
    if (inst->values.immediate_bytes_count > 3) {
        sprintf(num, "%02x", (unsigned char)inst->values.immediate[3]);
        memcpy(buffer + 67, num, 2);
    }
    str_cats(s, buffer);
}
