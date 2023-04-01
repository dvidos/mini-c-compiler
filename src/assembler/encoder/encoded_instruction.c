#include <string.h>
#include <stdio.h>
#include "encoded_instruction.h"


void pack_encoded_instruction(struct encoded_instruction *inst, char *buffer, int *buffer_length) {

    // prefixes
    if (inst->flags.have_instruction_prefix) {
        *buffer++ = inst->instruction_prefix;
        (*buffer_length)++;
    }
    if (inst->flags.have_address_size_prefix) {
        *buffer++ = inst->address_size_prefix;
        (*buffer_length)++;
    }
    if (inst->flags.have_operand_size_prefix) {
        *buffer++ = inst->operand_size_prefix;
        (*buffer_length)++;
    }
    if (inst->flags.have_segment_override_prefix) {
        *buffer++ = inst->segment_override_prefix;
        (*buffer_length)++;
    }

    // opcode(s)
    if (inst->flags.have_opcode_expansion_byte) {
        *buffer++ = inst->opcode_expansion_byte;
        (*buffer_length)++;
    }
    *buffer++ = inst->opcode_byte;
    (*buffer_length)++;

    // operands
    if (inst->flags.have_modregrm) {
        *buffer++ = inst->modregrm_byte;
        (*buffer_length)++;
    }
    if (inst->flags.have_sib) {
        *buffer++ = inst->sib_byte;
        (*buffer_length)++;
    }

    // displacement & immediate
    if (inst->displacement_bytes_count > 0) {
        memcpy(buffer, inst->displacement, inst->displacement_bytes_count);
        buffer += inst->displacement_bytes_count;
        (*buffer_length) += inst->displacement_bytes_count;
    }
    if (inst->immediate_bytes_count > 0) {
        memcpy(buffer, inst->immediate, inst->immediate_bytes_count);
        buffer += inst->immediate_bytes_count;
        (*buffer_length) += inst->immediate_bytes_count;
    }
}

void print_encoded_instruction(struct encoded_instruction *inst, FILE *stream) {
/*
    4 prefix bytes, 2 opcode bytes, modregrm, sib, 0-4 displacement, 0-4 immediate

    "Ins Adr Opr Seg   Opc         ModR/M       SIB                                "
    "Pfx Siz Siz Ovr   Pfx Opc   Md Reg R/M  SS Idx Bse   Displacemnt   Immediate  "
    " 00  --  00  --    00  00   00.000.000  00.000.000   00 00 00 00   00 00 -- --"
*/
    char num[16];
    char buffer[128];

    if (inst == NULL) {
        fprintf(stream, "Ins Adr Opr Seg   Opc         ModR/M       SIB                                \n");
        fprintf(stream, "Pfx Siz Siz Ovr   Pfx Opc   Md Reg R/M  SS Idx Bse   Displacemnt   Immediate  \n");
        return;
    }
    //                   0         1         2         3         4         5         6         7         8
    //                   012345678901234567890123456789012345678901234567890123456789012345678901234567890
    strcpy(buffer,      " --  --  --  --    --  --   --.---.---  --.---.---   -- -- -- --   -- -- -- --");

    if (inst->flags.have_instruction_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->instruction_prefix);
        memcpy(buffer + 1, num, 2);
    }
    if (inst->flags.have_address_size_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->address_size_prefix);
        memcpy(buffer + 5, num, 2);
    }
    if (inst->flags.have_operand_size_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->operand_size_prefix);
        memcpy(buffer + 9, num, 2);
    }
    if (inst->flags.have_segment_override_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->segment_override_prefix);
        memcpy(buffer + 13, num, 2);
    }
    if (inst->flags.have_opcode_expansion_byte) {
        sprintf(num, "%02x", (unsigned char)inst->opcode_expansion_byte);
        memcpy(buffer + 19, num, 2);
    }
    sprintf(num, "%02x", (unsigned char)inst->opcode_byte);
    memcpy(buffer + 23, num, 2);

    if (inst->flags.have_modregrm) {
        sprintf(num, "%d%d.%d%d%d.%d%d%d", 
            (inst->modregrm_byte >> 7) & 0x1,
            (inst->modregrm_byte >> 6) & 0x1,
            (inst->modregrm_byte >> 5) & 0x1,
            (inst->modregrm_byte >> 4) & 0x1,
            (inst->modregrm_byte >> 3) & 0x1,
            (inst->modregrm_byte >> 2) & 0x1,
            (inst->modregrm_byte >> 1) & 0x1,
            (inst->modregrm_byte >> 0) & 0x1);
        memcpy(buffer + 28, num, 10);
    }
    if (inst->flags.have_sib) {
        sprintf(num, "%d%d.%d%d%d.%d%d%d", 
            (inst->sib_byte >> 7) & 0x1,
            (inst->sib_byte >> 6) & 0x1,
            (inst->sib_byte >> 5) & 0x1,
            (inst->sib_byte >> 4) & 0x1,
            (inst->sib_byte >> 3) & 0x1,
            (inst->sib_byte >> 2) & 0x1,
            (inst->sib_byte >> 1) & 0x1,
            (inst->sib_byte >> 0) & 0x1);
        memcpy(buffer + 40, num, 10);
    }
    if (inst->displacement_bytes_count > 0) {
        sprintf(num, "%02x", (unsigned char)inst->displacement[0]);
        memcpy(buffer + 53, num, 2);
    }
    if (inst->displacement_bytes_count > 1) {
        sprintf(num, "%02x", (unsigned char)inst->displacement[1]);
        memcpy(buffer + 56, num, 2);
    }
    if (inst->displacement_bytes_count > 2) {
        sprintf(num, "%02x", (unsigned char)inst->displacement[2]);
        memcpy(buffer + 59, num, 2);
    }
    if (inst->displacement_bytes_count > 3) {
        sprintf(num, "%02x", (unsigned char)inst->displacement[3]);
        memcpy(buffer + 62, num, 2);
    }
    if (inst->immediate_bytes_count > 0) {
        sprintf(num, "%02x", (unsigned char)inst->immediate[0]);
        memcpy(buffer + 67, num, 2);
    }
    if (inst->immediate_bytes_count > 1) {
        sprintf(num, "%02x", (unsigned char)inst->immediate[1]);
        memcpy(buffer + 70, num, 2);
    }
    if (inst->immediate_bytes_count > 2) {
        sprintf(num, "%02x", (unsigned char)inst->immediate[2]);
        memcpy(buffer + 73, num, 2);
    }
    if (inst->immediate_bytes_count > 3) {
        sprintf(num, "%02x", (unsigned char)inst->immediate[3]);
        memcpy(buffer + 76, num, 2);
    }
    fprintf(stream, "%s\n", buffer);
}