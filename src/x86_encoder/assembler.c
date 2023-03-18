#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../err_handler.h"
#include "../options.h"
#include "../codegen/ir_listing.h"
#include "obj_code.h"
#include "asm_listing.h"
#include "encoder.h"
#include "asm_allocator.h"




static struct function_assembling_information {
    asm_listing *lst;

    // as we assemble each function
    struct ir_entry_func_def_info *func_def;
    int stack_space_for_local_vars;
} f;



static void analyze_function(ir_listing *ir, int start, int end) {
    if (ir->entries_arr[start]->type != IR_FUNCTION_DEFINITION) {
        error(NULL, 0, "internal bug, function declaration IR was expected");
        return;
    }
    f.func_def = &ir->entries_arr[start]->t.function_def;
    asm_allocator.reset();
    
    // declare stack variables and their offsets from BP:
    //   BP + n = last argument (that was pushed first, right-to-left)
    //   BP + 8 = first argument (that was pushed last of the arguments)
    //   BP + 4 = return address (that was pushed last)
    //   BP + 0 = previous BP
    //   BP - 4 = first local variable
    //   BP - n = local variable
    // this allows the allocator to grab more stack space as needed

    int bp_offset = options.pointer_size_bytes * 2; // skip pushed EBP and return address
    for (int i = 0; i < f.func_def->args_len; i++) {
        asm_allocator.declare_local_symbol(
            f.func_def->args_arr[i].name, f.func_def->args_arr[i].size,
            bp_offset);
        bp_offset += f.func_def->args_arr[i].size;
    }

    bp_offset = 0; // to subtract the size of the first local variable, not of BP
    f.stack_space_for_local_vars = 0;
    for (int i = start; i < end; i++) {
        ir_entry *e = ir->entries_arr[i];
        if (e->type == IR_DATA_DECLARATION && e->t.data_decl.storage == IR_LOCAL) {
            bp_offset -= e->t.data_decl.size; // note we subtract before
            asm_allocator.declare_local_symbol(
                e->t.data_decl.symbol_name, e->t.data_decl.size,
                bp_offset);
            f.stack_space_for_local_vars += e->t.data_decl.size;
        }
    }
}

static void code_prologue() {
    f.lst->ops->set_next_label(f.lst, f.func_def->func_name);
    f.lst->ops->add_instr_reg(f.lst, OC_PUSH, REG_BP);
    f.lst->ops->add_instr_reg_reg(f.lst, OC_MOV, REG_BP, REG_SP);
    if (f.stack_space_for_local_vars > 0) {
        f.lst->ops->add_comment(f.lst, "space for local vars", true);
        f.lst->ops->add_instr_reg_imm(f.lst, OC_SUB, REG_SP, f.stack_space_for_local_vars);
    }

    asm_allocator.generate_stack_info_comments();

    // then allocate local variables (and temp regs as well)
    // callee saved registers
    // CX and AX are clobbered during a call
}

static void code_epilogue() {
    f.lst->ops->add_instr_reg_reg(f.lst, OC_MOV, REG_SP, REG_BP);
    f.lst->ops->add_instr_reg(f.lst, OC_POP, REG_BP);
    // callee restored registers
    // must move ret_val to EAX
    f.lst->ops->add_instr(f.lst, OC_RET);
}

static void code_function_call(struct ir_entry_function_call_info *c) {
    // storage s;
    // int bytes_pushed = 0;

    // // caller saved registers
    // // push arguments from right to left
    // // clean up pushed arguments (e.g. ADD SP, 8)
    // // caller restore registers

    // for (int i = c->args_len - 1; i >= 0; i--) {
    //     ir_value *v = c->args_arr[i];
    //     if (v->type == IR_IMM) {
    //         f.lst->ops->add_instr_imm(f.lst, OC_PUSH, v->val.immediate);
    //     } else if (v->type == IR_SYM) {
    //         // resolve possible local variables
    //         if (asm_allocator.get_named_storage(v->val.symbol_name, &s)) {
    //             if (s.is_gp_reg)
    //                 f.lst->ops->add_instr_reg(f.lst, OC_PUSH, s.gp_reg);
    //             else if (s.is_stack_var)
    //                 f.lst->ops->add_instr_reg(f.lst, OC_PUSH, REG_BP); // bp_offset??????
    //         } else {
    //             // it should be a global variable, let linker resolve this
    //             f.lst->ops->add_instr_sym(f.lst, OC_PUSH, v->val.symbol_name);
    //         }
    //     } else if (v->type == IR_REG) {
    //         asm_allocator.get_temp_reg_storage(v->val.reg_no, &s);
    //         if (s.is_gp_reg)
    //             f.lst->ops->add_instr_reg(f.lst, OC_PUSH, s.gp_reg);
    //         else if (s.is_stack_var)
    //             f.lst->ops->add_instr_reg(f.lst, OC_PUSH, REG_BP); // bp_offset??????
    //     }
    //     bytes_pushed += options.pointer_size_bytes; // how can we be sure?
    // }

    // // then call the address
    // if (c->func_addr->type == IR_IMM) {
    //     f.lst->ops->add_instr_imm(f.lst, OC_CALL, v->val.immediate);
    // } else if (c->func_addr->type == IR_SYM) {
    //     // can this be a local symbol? a variable where we assign a function address
    //     if (asm_allocator.get_named_storage(c->func_addr->val.symbol_name, &storage)) {
    //         if (s.is_gp_reg)
    //             f.lst->ops->add_instr_reg(f.lst, OC_CALL, s.gp_reg);
    //         else if (s.is_stack_var)
    //             f.lst->ops->add_instr_reg(f.lst, OC_CALL, REG_BP); // bp_offset??????
    //     } else {
    //         // not local, let linker figure this out
    //         f.lst->ops->add_instr_sym(f.lst, OC_CALL, c->func_addr->val.symbol_name);
    //     }
    // } else if (c->func_addr->type == IR_REG) {
    //     // i guess temp reg holds the target address?
    //     asm_allocator.get_temp_reg_storage(c->func_addr->val.reg_no, &s);
    //     if (s.is_gp_reg)
    //         f.lst->ops->add_instr_reg(f.lst, OC_PUSH, s.gp_reg);
    //     else if (s.is_stack_var)
    //         f.lst->ops->add_instr_reg(f.lst, OC_PUSH, REG_BP); // bp_offset??????
    // }

    // // then grab return value
    // if (c->lvalue != NULL) {
    //     if (c->lvalue->type == IR_SYM) {
    //         // resolve possible local variables
    //         if (asm_allocator.get_named_storage(c->lvalue->val.symbol_name, &s)) {
    //             if (s.is_gp_reg)
    //                 f.lst->ops->add_instr_reg_reg(f.lst, OC_MOV, s.gp_reg, REG_AX);
    //             else if (s.is_stack_var)
    //                 f.lst->ops->add_instr_reg(f.lst, OC_MOV, REG_BP, REG_AX); // bp_offset??????
    //         } else {
    //             // it should be a global variable, let linker resolve this
    //             f.lst->ops->add_instr_sym(f.lst, OC_MOV, v->val.symbol_name, REG_AX);
    //         }
    //     } else if (v->type == IR_REG) {
    //         asm_allocator.get_temp_reg_storage(v->val.reg_no, &s);
    //         if (s.is_gp_reg)
    //             f.lst->ops->add_instr_reg(f.lst, OC_PUSH, s.gp_reg, REG_AX);
    //         else if (s.is_stack_var)
    //             f.lst->ops->add_instr_reg(f.lst, OC_PUSH, REG_BP, REG_AX); // bp_offset??????
    //     }

    // }

    // // clean up pushed values
    // f.lst->ops->add_instr_reg_imm(f.lst, OC_ADD, REG_SP, bytes_pushed);
}

static void code_conditional_jump(struct ir_entry_cond_jump_info *j) {
    // emit two things: compare, then appropriate jump.
    // we also need to clobber at least one register for CMP
    // the second _may_ be an immediate
    f.lst->ops->add_instr_reg_reg(f.lst, OC_CMP, REG_AX, REG_DX);
    enum opcode op;
    switch (j->cmp) {
        case IR_EQ: op = OC_JEQ; break;
        case IR_NE: op = OC_JNE; break;
        case IR_GT: op = OC_JGT; break;
        case IR_GE: op = OC_JGE; break;
        case IR_LT: op = OC_JLT; break;
        case IR_LE: op = OC_JLE; break;
    }
    f.lst->ops->add_instr_sym(f.lst, op, j->target_label);
}

static void code_unconditional_jump(char *label) {
    f.lst->ops->add_instr_sym(f.lst, OC_JMP, label);  // if everything was as simple!
}

static void code_three_address_code(struct ir_entry_three_addr_code_info *c) {
    // maybe each one can be either a register, or a BP-related temp var.
    // we grab it, then we release it, if we can.
}

static void assemble_function(ir_listing *ir, int start, int end) {
    // also see https://courses.cs.washington.edu/courses/cse401/06sp/codegen.pdf
    // we use asm_allocator to allocate storage space for temp registers
    // this storage space can be either CPU registers or stack space.
    // for each IR instruction, find appropriate assembly instruction(s)

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
                f.lst->ops->set_next_label(f.lst, e->t.label.str);
                break;
            case IR_DATA_DECLARATION:
                // how about initial data?
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
}


// given an Intemediate Representation listing, generate an assembly listing.
void x86_assemble_ir_listing(ir_listing *ir_list, asm_listing *asm_list) {

    // set reference to asm listing, to allow our functions to use it
    f.lst = asm_list;
    asm_allocator.set_asm_listing(asm_list);

    // calculate temp register usage and last mention
    ir_list->ops->run_statistics(ir_list);

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

