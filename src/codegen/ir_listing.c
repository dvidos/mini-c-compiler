#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "ir_listing.h"


static void _add(ir_listing *l, ir_entry *entry);
static void _print(ir_listing *l, FILE *stream);
static void _free(ir_listing *l);

static struct ir_listing_ops ops = {
    .add = _add,
    .print = _print,
    .free = _free
};

ir_listing *new_ir_listing() {
    ir_listing *l = malloc(sizeof(ir_listing));
    l->capacity = 10;
    l->length = 0;
    l->entries_arr = malloc(sizeof(ir_listing *) * l->capacity);
    l->ops = &ops;
    return l;
}

static void _add(ir_listing *l, ir_entry *entry) {
    if (l->length + 1 >= l->capacity) {
        l->capacity *= 2;
        l->entries_arr = realloc(l->entries_arr, sizeof(ir_listing *) * l->capacity);
    }

    l->entries_arr[l->length] = entry;
    l->length++;
}

static void _print(ir_listing *l ,FILE *stream) {
    for (int i = 0; i < l->length; i++) {
        ir_entry *e = l->entries_arr[i];
        e->ops->print(e, stream);
        fprintf(stream, "\n");
    }
}

static void _free(ir_listing *l) {
    for (int i = 0; i < l->length; i++) {
        ir_entry *e = l->entries_arr[i];
        e->ops->free(e);
    }
    free(l->entries_arr);
    free(l);
}

/*
    Running environment in IA-32:
        - access to 4 GB of memory
        Program execution registers:
        - eight general purpose registers (EAX, EBX, ECX, EDX, ESI, EDI, EBP, ESP)
        - six segment registers (16bits) (CS, DS, SS, ES, FS, GS)
        - 32-bit instruction pointer EIP, 32-bit EFLAGS register
        These do integer arithmetic, string manipulation, flow control etc.
        Extra registers: FPU, MMX, XMM, YMM, BND etc for floating, boudaries etc
        I/O ports
        Status registers (CR0 - CR3)
        Memory managmeent registers (GDTR, IDTR, LDTR etc)
        Debug registers (DR0 - DR7)

    Running environment in IA-64:
        - access to 17179869183 GB of memory...
        Program execution registers:
        - eight general purpose registers (RAX, RBX, RCX, RDX, RSI, RDI, RBP, RSP)
        - six segment registers (CS, DS, SS, ES, FS, GS)
        - 64-bit instruction pointer RIP, 64-bit RFLAGS register
        These do integer arithmetic, string manipulation, flow control etc.
        Extra registers: FPU, MMX, XMM, YMM, BND etc for floating, boudaries etc
        I/O ports
        Status registers (CR0 - CR3)
        Memory managmeent registers (GDTR, IDTR, LDTR etc)
        Debug registers (DR0 - DR7)
*/
static void _generate_assembly(ir_listing *l) {

}