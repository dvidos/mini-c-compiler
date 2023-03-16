#include <stdlib.h>
#include <stddef.h>
#include "../err_handler.h"
#include "../options.h"
#include "../codegen/ir_listing.h"
#include "obj_code.h"
#include "asm_listing.h"
#include "encoder.h"


// AX...DI, each has a flag.
// ability to allocate and deallocate a register.
static int gen_register_state = 0; 
static int grab_reg() { // return an allocated register (0-7) or -1
    for (int i = 0; i < 8; i++) {
        if (i == REG_SP || i == REG_BP) // these two are special
            continue;
        if ((gen_register_state & (1 << i)) == 0) {
            gen_register_state |= (1 << i);
            return i;
        }
    }
    return -1;
}
static void release_reg(int reg_no) {
    gen_register_state &= ~(1 << reg_no);
}



// we need to track and allocate registers, local vars and arguments etc.
struct data_info {
    // can be global data,code label = leave as symbol
    // can be argument, return BP offset + size
    // can be local variable, return BP offset + size
    // can be temp register, either GP register, or stack space + size
};
struct stack_var {
    int bp_offset; // from -1K to +1K
    char *name;    // argument, or local var
    int reg_no;    // if name == NULL, this is a register
    int size;      // 1, 2, 4, 8 bytes
};
struct function_assembling_information {
    // register allocation information,
    // stack allocation information
    // all variables location and addressing information
    asm_listing *al;

    struct stack_var *stack_vars;
    int stack_vars_capacity;
    int stack_vars_length;
};

static struct function_assembling_information ai;

static void add_stack_var(int bp_offset, char *name, int reg_no, int size) {
    if (ai.stack_vars_length + 1 >= ai.stack_vars_capacity) {
        ai.stack_vars_capacity *= 2;
        ai.stack_vars = realloc(ai.stack_vars, sizeof(struct stack_var) * ai.stack_vars_capacity);
    }
    struct stack_var *v = &ai.stack_vars[ai.stack_vars_length];
    v->bp_offset = bp_offset;
    v->name = name;
    v->reg_no = reg_no;
    v->size = size;
    ai.stack_vars_length++;
}

static void analyze_function(ir_listing *ir, int start, int end) {
    char buffer[64];

    // analyze function, maybe for each temp register, 
    // decide if a register will be used (and which)
    // or if stack space will be used (and which)
    // allocate all that will be allocated on stack
    // and prepare a stack map, essentially.
    // a technique is to measure the life time of a temp register,
    // and after it is not needed, one can use the allocated storage for another temp register
    // essentially, measure the amount of temporary storages we need, and map each temp reg to one.

    ai.stack_vars_capacity = 16;
    ai.stack_vars_length = 0;
    ai.stack_vars = malloc(sizeof(struct stack_var) * ai.stack_vars_capacity);
    
    // stack frame:
    //   BP + n = last argument (that was pushed first, right-to-left)
    //   BP + 8 = first argument (that was pushed last of the arguments)
    //   BP + 4 = return address (that was pushed last)
    //   BP + 0 = previous BP
    //   BP - 4 = first local variable
    //   BP - n = local variable
    struct ir_entry_func_def_info *func_def = &ir->entries_arr[start]->t.function_def;
    int bp_offset = options.pointer_size_bytes * 2;
    for (int i = 0; i < func_def->args_len; i++) {
        add_stack_var(bp_offset, func_def->args_arr[i].name, 0, func_def->args_arr[i].size);
        bp_offset += func_def->args_arr[i].size;
    }
    bp_offset = 0; // to subtract the size of the local variable, not of BP
    for (int i = start; i < end; i++) {
        ir_entry *e = ir->entries_arr[i];
        if (e->type == IR_DATA_DECLARATION && e->t.data_decl.storage == IR_LOCAL) {
            bp_offset -= e->t.data_decl.size;
            add_stack_var(bp_offset, e->t.data_decl.symbol_name, 0, e->t.data_decl.size);
        }
    }

    // add the stack frame to assembly listing
    ai.al->ops->add_comment(ai.al, buffer, "Stack Frame");
    for (int i = 0; i < ai.stack_vars_length; i++) {
        sprintf(buffer, "    [%cBP %s%d] %s \"%s\" (%d bytes)",
            options.register_prefix,
            ai.stack_vars[i].bp_offset < 0 ? "" : "+",
            ai.stack_vars[i].bp_offset,
            ai.stack_vars[i].bp_offset < 0 ? "local" : "arg",
            ai.stack_vars[i].name,
            ai.stack_vars[i].size);
        ai.al->ops->add_comment(ai.al, buffer, false);
    }

}

static void clean_up_function_analysis() {
    ai.stack_vars_capacity = 0;
    free(ai.stack_vars);
}

static void code_prologue() {
    ai.al->ops->add_instr_reg(ai.al, OC_PUSH, REG_BP);
    ai.al->ops->add_instr_reg_reg(ai.al, OC_MOV, REG_BP, REG_SP);
    // then allocate local variables (and temp regs as well)
    // callee saved registers
}

static void code_epilogue() {
    ai.al->ops->add_instr_reg_reg(ai.al, OC_MOV, REG_SP, REG_BP);
    ai.al->ops->add_instr_reg(ai.al, OC_POP, REG_BP);
    // what other clean up
    // callee restored registers
}

static void code_function_call(struct ir_entry_function_call_info *c) {
    // caller saved registers
    // push arguments from right to left
    // it depends in the type of address, it may be a symbolname or a reg with the address
    // ai.al->ops->add_instr_sym(ai.al, OC_CALL, NULL);
    // clean up pushed arguments (e.g. ADD SP, 8)
    // caller restore registers
}

static void code_conditional_jump(struct ir_entry_cond_jump_info *j) {
    // emit two things: compare, then appropriate jump.
    // we also need to clobber at least one register for CMP
    // the second _may_ be an immediate
}

static void code_unconditional_jump(char *label) {
    ai.al->ops->add_instr_sym(ai.al, OC_JMP, label);  // if everything was as simple!
}

static void code_three_address_code(struct ir_entry_three_addr_code_info *c) {
    // maybe each one can be either a register, or a BP-related temp var.
    // we grab it, then we release it, if we can.
}

static void assemble_function(ir_listing *ir, int start, int end) {
    // also see https://courses.cs.washington.edu/courses/cse401/06sp/codegen.pdf
    // this function owns stack usage, register usage and function calling contract
    // analyze usage of registers, return patterns etc.
    // for each temp register, select register or stack location to use.
    // reuse registers as much as possible
    // compute stack layout for all variables, args, locals and extra ones
    // for each IR instruction, find appropriate assembly instruction(s)
    // some registers are dedicated: BP, SP.
    // keep a couple of registers for temp use, i.e. load from memory to work on
    // must decide what registers are caller saved vs callee saved, 
    //     as maybe nested function calls may clobber them.
    // ax, bx, cx, dx = allocatable.
    // si, di = scratch registers (temp usage)
    // sp = stack, bp = stack frame.
    // some operations (e.g. ADD X, Y) can immediately deallocate the Y register

    analyze_function(ir, start, end);
    code_prologue();

    for (int i = start; i < end; i++) {
        ir_entry *e = ir->entries_arr[i];
        switch (e->type) {
            case IR_FUNCTION_DEFINITION: // fallthrough
            case IR_COMMENT:
                // ignore
                break;
            case IR_LABEL:
                ai.al->ops->set_next_label(ai.al, e->t.label.str);
                break;
            case IR_DATA_DECLARATION:
                // grab more stack space?
                break;
            case IR_THREE_ADDR_CODE:
                code_three_address_code(&e->t.three_address_code);
                break;
            case IR_FUNCTION_CALL:
                code_function_call(&e->t.function_call);
                break;
            case IR_CONDITIONAL_JUMP:
                code_conditional_jump(&e->t.conditional_jump);
                break;
            case IR_UNCONDITIONAL_JUMP:
                code_unconditional_jump(e->t.unconditional_jump.str);
                break;
        }
    }

    code_epilogue();
    clean_up_function_analysis();
}


// given an Intemediate Representation listing, generate an assembly listing.
void x86_assemble_ir_listing(ir_listing *ir_list, asm_listing *asm_list) {

    ai.al = asm_list;

    // emit assembly code to reserve .data, .bss and .rodata
    // db, dw, dd, dq etc.

    // do one function at a time
    int start = 0;
    while (start < ir_list->length) {
        start = ir_list->ops->find_next_function_def(ir_list, start);
        if (start == -1) break;

        // non-inclusive upper bound.
        int end = ir_list->ops->find_next_function_def(ir_list, start + 1);
        if (end == -1)
            end = ir_list->length;

        assemble_function(ir_list, start, end);
        start = end;
    }
}

void x86_encode_asm_into_machine_code(asm_listing *asm_list, enum x86_cpu_mode mode, obj_code *mod) {
    // encode this into intel machine code
    struct x86_encoder *enc = new_x86_encoder(mode, mod->text_seg, mod->relocations);
    struct instruction *inst;

    for (int i = 0; i < asm_list->length; i++) {
        inst = &asm_list->instructions[i];

        if (inst->label != NULL) {
            // we don't know if this is exported for now
            mod->symbols->add(mod->symbols, inst->label, mod->text_seg->length, SB_CODE);
        }

        if (!enc->encode(enc, inst)) {
            char str[128];
            instruction_to_string(inst, str, sizeof(str));
            error(NULL, 0, "Failed encoding instruction: '%s'\n", str);
            return;
        }
    }
}

