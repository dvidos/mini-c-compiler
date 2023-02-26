#include <stdio.h>
#include <string.h>
#include "instruction.h"
#include "encoder.h"
#include "bin_buffer.h"
#include "../utils.h"


void perform_asm_test() {
    // we should really do this one: https://www.tutorialspoint.com/assembly_programming/assembly_system_calls.htm

    
    /*
        401000:	b8 34 12 00 00       	mov    $0x1234,%eax
        401005:	b8 01 00 00 00       	mov    $0x1,%eax
        40100a:	bb 08 00 00 00       	mov    $0x8,%ebx
        40100f:	cd 80                	int    $0x80
    */
    struct instruction list[16];

    // 0:  50                      push   eax
    // 1:  ff 33                   push   DWORD PTR [ebx]
    // 3:  6a 7b                   push   0x7b
    // 5:  ff 35 14 00 00 00       push   DWORD PTR ds:0x14
    // b:  59                      pop    ecx
    // c:  8f 02                   pop    DWORD PTR [edx]
    // e:  8f 05 14 00 00 00       pop    DWORD PTR ds:0x14
    
    int count = 0;
    memset(&list, 0, sizeof(list));

    list[count].opcode = OC_PUSH;
    list[count].op1.type = OT_REGISTER;
    list[count].op1.value = REG_AX;
    count++;

    list[count].opcode = OC_PUSH;
    list[count].op1.type = OT_MEMORY_POINTED_BY_REG;
    list[count].op1.value = REG_BX;
    count++;

    list[count].opcode = OC_PUSH;
    list[count].op1.type = OT_IMMEDIATE;
    list[count].op1.value = 0x1234;
    count++;

    list[count].opcode = OC_PUSH;
    list[count].op1.type = OT_MEMORY_POINTED_BY_SYMBOL;
    list[count].op1.symbol_name = "nicholas";
    count++;

    list[count].opcode = OC_POP;
    list[count].op1.type = OT_REGISTER;
    list[count].op1.value = REG_CX;
    count++;

    list[count].opcode = OC_POP;
    list[count].op1.type = OT_MEMORY_POINTED_BY_REG;
    list[count].op1.value = REG_DX;
    count++;

    list[count].opcode = OC_POP;
    list[count].op1.type = OT_MEMORY_POINTED_BY_SYMBOL;
    list[count].op1.symbol_name = "nicholas";
    count++;

    // list[1].opcode = OC_MOV;
    // list[1].op1.type = OT_REGISTER;
    // list[1].op1.value = REG_AX;
    // list[1].op2.type = OT_IMMEDIATE;
    // list[1].op2.value = 1;

    // list[2].opcode = OC_MOV;
    // list[2].op1.type = OT_REGISTER;
    // list[2].op1.value = REG_BX;
    // list[2].op2.type = OT_IMMEDIATE;
    // list[2].op2.value = 8;

    // list[3].opcode = OC_INT;
    // list[3].op1.type = OT_IMMEDIATE;
    // list[3].op1.value = 0x80;

    struct x86_encoder *encoder = new_x86_encoder(CPU_MODE_PROTECTED);
    for (int i = 0; i < count; i++) {
        encoder->encode(encoder, &list[i]);
    }

    printf("Assembly code listing:\n");
    for (int i = 0; i < count; i++)
        print_instruction("\t", &list[i]);

    printf("Encoded machine code: (%d bytes)\n", encoder->output->length);
    print_16_hex(encoder->output->data, encoder->output->length);

    printf("Copyable format: ");
    for (int i = 0; i < encoder->output->length; i++)
        printf("%02x", (unsigned char)encoder->output->data[i]);
    printf("\n");


    printf("Unresolved references:\n");
    printf("  Position  Symbol name\n");
    //        12345678  abcdef
    for (int i = 0; i < encoder->references->count; i++) {
        struct reference *r = &encoder->references->references[i];
        printf("  %8ld  %s\n", r->position, r->name);
    }
}
