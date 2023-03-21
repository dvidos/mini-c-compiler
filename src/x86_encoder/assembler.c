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
#include "asm_instruction.h"


// for converting temp registers and local symbols to assembly operands
struct asm_operand *ir_value_to_asm_operand(ir_value *v) {
    // consult temp register allocation, to resolve temp reg numbers
    // consult local symbols table, to resolve local symbols (e.g. func arguments)
    // immediates stay as they are
    // global symbols stay as they are
    storage s;
    struct asm_operand *o = malloc(sizeof(struct asm_operand));

    if (v->type == IR_TREG) {
        asm_allocator.get_temp_reg_storage(v->val.temp_reg_no, &s);
        if (s.is_gp_reg) {
            o->type = OT_REGISTER;
            o->reg = s.gp_reg;
        } else if (s.is_stack_var) {
            o->type = OT_MEM_POINTED_BY_REG;
            o->reg = REG_BP;
            o->offset = s.bp_offset;
        }
    } else if (v->type == IR_SYM) {
        if (asm_allocator.get_named_storage(v->val.symbol_name, &s)) {
            if (!s.is_stack_var) {
                error(NULL, 0, "named symbols ('%s') are expected to be stack oriented", v->val.symbol_name);
                return NULL;
            }
            o->type = OT_MEM_POINTED_BY_REG;
            o->reg = REG_BP;
            o->offset = s.bp_offset;
        } else {
            // not found in local symbols, let linker sort this out
            o->type = OT_MEM_OF_SYMBOL;
            o->symbol_name = v->val.symbol_name;
        }
    } else if (v->type == IR_IMM) {
        o->type = OT_IMMEDIATE;
        o->immediate = v->val.immediate;
    } else {
        o->type = OT_NONE;
    }

    return o;
}



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
    f.lst->ops->add_comment(f.lst, true, "establish stak frame");
    f.lst->ops->add_instr1(f.lst, OC_PUSH, new_reg_asm_operand(REG_BP));
    f.lst->ops->add_instr2(f.lst, OC_MOV, new_reg_asm_operand(REG_BP), new_reg_asm_operand(REG_SP));
    if (f.stack_space_for_local_vars > 0) {
        f.lst->ops->add_comment(f.lst, true, "space for local vars");
        f.lst->ops->add_instr2(f.lst, OC_SUB, new_reg_asm_operand(REG_SP), new_imm_asm_operand(f.stack_space_for_local_vars));
    }

    asm_allocator.generate_stack_info_comments();

    // then allocate local variables (and temp regs as well)
    // callee saved registers
    // CX and AX are clobbered during a call
}

static void code_epilogue() {
    char label_name[128];
    label_name[sizeof(label_name) - 1] = '\0';
    snprintf(label_name, sizeof(label_name) - 1, "%s_exit", f.func_def->func_name);
    f.lst->ops->set_next_label(f.lst, label_name);

    f.lst->ops->add_comment(f.lst, true, "tear down stak frame");
    f.lst->ops->add_instr2(f.lst, OC_MOV, new_reg_asm_operand(REG_SP), new_reg_asm_operand(REG_BP));
    f.lst->ops->add_instr1(f.lst, OC_POP, new_reg_asm_operand(REG_BP));
    f.lst->ops->add_instr(f.lst, OC_RET);
}

static void code_function_call(struct ir_entry_function_call_info *c) {
    int bytes_pushed = 0;

    // caller saved registers

    // push arguments from right to left
    if (c->args_len > 0) 
        f.lst->ops->add_comment(f.lst, true, "push %d args for function call", c->args_len);
    
    for (int i = c->args_len - 1; i >= 0; i--) {
        ir_value *v = c->args_arr[i];
        f.lst->ops->add_instr1(f.lst, OC_PUSH, ir_value_to_asm_operand(v));
        bytes_pushed += options.pointer_size_bytes; // how can we be sure?
    }
    f.lst->ops->add_instr1(f.lst, OC_CALL, ir_value_to_asm_operand(c->func_addr));

    // grab returned value, if any is expected
    if (c->lvalue != NULL) {
        f.lst->ops->add_instr2(f.lst, OC_MOV, ir_value_to_asm_operand(c->lvalue), new_reg_asm_operand(REG_AX));
    }
    // clean up pushed arguments
    f.lst->ops->add_instr2(f.lst, OC_ADD, new_reg_asm_operand(REG_SP), new_imm_asm_operand(bytes_pushed));

    // caller restore registers (except AX)
}

static void code_conditional_jump(struct ir_entry_cond_jump_info *j) {
    // emit two things: compare, then appropriate jump.
    // we also need to clobber at least one register for CMP
    // the second _may_ be an immediate
    f.lst->ops->add_instr2(f.lst, OC_CMP, ir_value_to_asm_operand(j->v1), ir_value_to_asm_operand(j->v2));
    enum opcode op;
    switch (j->cmp) {
        case IR_EQ: op = OC_JEQ; break;
        case IR_NE: op = OC_JNE; break;
        case IR_GT: op = OC_JGT; break;
        case IR_GE: op = OC_JGE; break;
        case IR_LT: op = OC_JLT; break;
        case IR_LE: op = OC_JLE; break;
    }
    f.lst->ops->add_instr1(f.lst, op, new_mem_by_sym_asm_operand(j->target_label));
}

static void code_unconditional_jump(char *label) {
    f.lst->ops->add_instr1(f.lst, OC_JMP, new_mem_by_sym_asm_operand(label));
}

static void code_return_statement(struct ir_entry_return_info *info) {
    if (info->ret_val != NULL) {
        f.lst->ops->add_comment(f.lst, true, "put returned value in AX");
        f.lst->ops->add_instr2(f.lst, OC_MOV, 
            new_reg_asm_operand(REG_AX), 
            ir_value_to_asm_operand(info->ret_val));
    }

    char label_name[128];
    label_name[sizeof(label_name) - 1] = '\0';
    snprintf(label_name, sizeof(label_name) - 1, "%s_exit", f.func_def->func_name);
    f.lst->ops->add_instr1(f.lst, OC_JMP, new_mem_by_sym_asm_operand(label_name));
}

static void code_three_address_code(struct ir_entry_three_addr_code_info *c) {
    // sometimes we only have "c", which is a function call, ignoring ret value
    if (c->lvalue != NULL && c->op1 == NULL && c->op2 != NULL) {
        // we have a case of "lv = r2" or "lv = <unary> r2"
        switch (c->op) {
            case IR_NONE:
                f.lst->ops->add_instr2(f.lst, OC_MOV, ir_value_to_asm_operand(c->lvalue), ir_value_to_asm_operand(c->op2));
                break;
            case IR_NOT:
                // we can do this directly, using modRM byte
                f.lst->ops->add_instr1(f.lst, OC_NOT, ir_value_to_asm_operand(c->op2));
                break;
            case IR_NEG:
                // we can do this directly, using modRM byte
                f.lst->ops->add_instr1(f.lst, OC_NEG, ir_value_to_asm_operand(c->op2));
                break;
            case IR_ADDR_OF:
                f.lst->ops->add_instr1(f.lst, OC_LEA, ir_value_to_asm_operand(c->op2));
                break;
            case IR_VALUE_AT: // fallthrough, for I don't know how to do this.
            default:
                error(NULL, 0, "Unsupported 3-code-addr unary operator %d", c->op);
                break;
        }
    } else if (c->lvalue != NULL && c->op1 != NULL && c->op2 != NULL) {
        // lv = r1 + r2
        // thinking of doing: MOV AX, r1 / ADD AX, r2 / MOV lv, AX
        f.lst->ops->add_instr2(f.lst, OC_MOV, new_reg_asm_operand(REG_AX), ir_value_to_asm_operand(c->op1));
        switch (c->op) {
            case IR_ADD:
                f.lst->ops->add_instr2(f.lst, OC_ADD, new_reg_asm_operand(REG_AX), ir_value_to_asm_operand(c->op1));
                break;
            case IR_SUB:
                f.lst->ops->add_instr2(f.lst, OC_SUB, new_reg_asm_operand(REG_AX), ir_value_to_asm_operand(c->op1));
                break;
            case IR_MUL:
                f.lst->ops->add_instr2(f.lst, OC_IMUL, new_reg_asm_operand(REG_AX), ir_value_to_asm_operand(c->op1));
                break;
            case IR_DIV:
                f.lst->ops->add_instr2(f.lst, OC_IDIV, new_reg_asm_operand(REG_AX), ir_value_to_asm_operand(c->op1));
                break;
            case IR_MOD:
                // division puts remainder in DX
                f.lst->ops->add_instr2(f.lst, OC_IDIV, new_reg_asm_operand(REG_AX), ir_value_to_asm_operand(c->op1));
                f.lst->ops->add_instr2(f.lst, OC_MOV, new_reg_asm_operand(REG_AX), new_reg_asm_operand(REG_DX));
                break;
            case IR_AND:
                f.lst->ops->add_instr2(f.lst, OC_AND, new_reg_asm_operand(REG_AX), ir_value_to_asm_operand(c->op1));
                break;
            case IR_OR:
                f.lst->ops->add_instr2(f.lst, OC_OR, new_reg_asm_operand(REG_AX), ir_value_to_asm_operand(c->op1));
                break;
            case IR_XOR:
                f.lst->ops->add_instr2(f.lst, OC_XOR, new_reg_asm_operand(REG_AX), ir_value_to_asm_operand(c->op1));
                break;
            case IR_LSH:
                f.lst->ops->add_instr2(f.lst, OC_SHL, new_reg_asm_operand(REG_AX), ir_value_to_asm_operand(c->op1));
                break;
            case IR_RSH:
                f.lst->ops->add_instr2(f.lst, OC_SHR, new_reg_asm_operand(REG_AX), ir_value_to_asm_operand(c->op1));
                break;
            default:
                error(NULL, 0, "Unsupported 3-code-addr binary operator %d", c->op);
                break;
        }
        f.lst->ops->add_instr2(f.lst, OC_MOV, ir_value_to_asm_operand(c->lvalue), new_reg_asm_operand(REG_AX));
    } else {
        // error(NULL, 0, "unsupported 3-addr-code format: lv=%s, op1=%s, op2=%s", 
        //     c->lvalue == NULL ? "null" : "non-null",
        //     c->op1 == NULL    ? "null" : "non-null",
        //     c->op2 == NULL    ? "null" : "non-null");
    }
}

static void _release_temp_reg_allocations(ir_value *v, void *pdata, int idata) {
    ir_listing *ir = (ir_listing *)pdata;
    int curr_index = idata;

    if (v != NULL && v->type == IR_TREG) {
        int reg_no = v->val.temp_reg_no;
        if (ir->ops->get_register_last_usage(ir, reg_no) == curr_index) {
            asm_allocator.release_temp_reg_storage(reg_no);
        }
    }
    
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
            case IR_RETURN:
                code_return_statement(&e->t.return_stmt);
                break;
        }

        // must free temp reg allocations
        e->ops->visit_ir_values(e, _release_temp_reg_allocations, ir, i);
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
    struct asm_instruction *inst;

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

