#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../err_handler.h"
#include "../options.h"
#include "../utils/string.h"
#include "../compiler/codegen/ir_listing.h"
#include "../linker/obj_code.h"
#include "encoder/asm_listing.h"
#include "encoder/encoding_info.h"
#include "encoder/encoded_instruction.h"
#include "encoder/encoder.h"
#include "encoder/asm_allocator.h"
#include "encoder/asm_instruction.h"
#include "../utils/string.h"


static struct asm_operand *resolve_ir_value_to_asm_operand(ir_value *v);

static void code_prologue();
static void code_epilogue();
static void code_function_call(ir_entry *e);
static void code_conditional_jump(ir_entry *e);
static void code_unconditional_jump(ir_entry *e, char *label);
static void code_return_statement(ir_entry *e);
static void code_simple_assignment(ir_entry *e, ir_value *lvalue, ir_value *rvalue);
static void code_unary_operation(ir_entry *e, ir_value *lvalue, ir_operation op, ir_value *rvalue);
static void code_binary_operation(ir_entry *e, ir_value *lvalue, ir_value *rvalue1, ir_operation op, ir_value *rvalue2);
static void assemble_function(ir_listing *ir, int start, int end);


static struct assembler_data {
    asm_allocator *allocator;
    asm_listing *listing;

    // as we assemble each function
    struct ir_entry_func_def_info *func_def;
    int stack_space_for_local_vars;
} ad;


// for converting temp registers and local symbols to assembly operands
struct asm_operand *resolve_ir_value_to_asm_operand(ir_value *v) {
    struct asm_operand *o = malloc(sizeof(struct asm_operand));
    storage s;
    bool allocated;

    // consult temp register allocation, to resolve temp reg numbers
    // consult local symbols table, to resolve local symbols (e.g. func arguments)
    // immediates and global symbols stay as they are
    if (v->type == IR_TREG) {
        // could be either a register or stack value, depending on allocation
        ad.allocator->ops->get_temp_reg_storage(ad.allocator, v->val.temp_reg_no, &s, &allocated);
        if (allocated) {
            string *st = new_string();
            ad.allocator->ops->storage_to_str(ad.allocator, &s, st);
            ad.listing->ops->add_comment(ad.listing, "%s allocated to r%d", st->buffer, v->val.temp_reg_no);
            st->v->free(st);
        }
        if (s.is_gp_reg) {
            o->type = OT_REGISTER;
            o->reg = s.gp_reg;
        } else if (s.is_stack_var) {
            o->type = OT_MEM_POINTED_BY_REG;
            o->reg = REG_BP;
            o->offset = s.bp_offset;
        }
    } else if (v->type == IR_SYM) {
        // could be a local or global symbol
        if (ad.allocator->ops->get_named_storage(ad.allocator, v->val.symbol_name, &s)) {
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



static void code_prologue() {
    ad.listing->ops->set_next_label(ad.listing, ad.func_def->func_name);
    ad.listing->ops->set_next_comment(ad.listing, "establish stack frame");

    ad.listing->ops->add(ad.listing, new_asm_instruction_for_register(OC_PUSH, REG_BP));
    ad.listing->ops->add(ad.listing, new_asm_instruction_for_registers(OC_MOV, REG_BP, REG_SP));

    if (ad.stack_space_for_local_vars > 0) {
        ad.listing->ops->set_next_comment(ad.listing, "reserve %d bytes for local vars", ad.stack_space_for_local_vars);
        ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_SUB, 
            new_asm_operand_reg(REG_SP),
            new_asm_operand_imm(ad.stack_space_for_local_vars)));
    }

    ad.allocator->ops->generate_stack_info_comments(ad.allocator);

    // callee saved registers
    // CX and AX are clobbered during a call
}

static void code_epilogue() {
    ad.listing->ops->set_next_label(ad.listing, "%s_exit", ad.func_def->func_name);
    ad.listing->ops->set_next_comment(ad.listing, "tear down stack frame");
    ad.listing->ops->add(ad.listing, new_asm_instruction_for_registers(OC_MOV, REG_SP, REG_BP));
    ad.listing->ops->add(ad.listing, new_asm_instruction_for_register(OC_POP, REG_BP));
    ad.listing->ops->add(ad.listing, new_asm_instruction(OC_RET));
}

static void code_function_call(struct ir_entry *e) {
    struct ir_entry_function_call_info *c = &e->t.function_call;
    string *s = new_string();
    e->ops->to_string(e, s);
    ad.listing->ops->set_next_comment(ad.listing, "IR: %s", s->buffer);
    s->v->free(s);


    // caller saved registers


    // allocate for returned value, if any is expected, before pushing
    struct asm_operand *lval;
    if (c->lvalue != NULL) {
        lval = resolve_ir_value_to_asm_operand(c->lvalue);
    }
    
    int bytes_pushed = 0;
    for (int i = c->args_len - 1; i >= 0; i--) {
        struct asm_operand *op = resolve_ir_value_to_asm_operand(c->args_arr[i]);
        ad.listing->ops->add(ad.listing, new_asm_instruction_with_operand(OC_PUSH, op));
        bytes_pushed += options.pointer_size_bytes; // how can we be sure?
    }

    struct asm_operand *addr = resolve_ir_value_to_asm_operand(c->func_addr);
    ad.listing->ops->add(ad.listing, new_asm_instruction_with_operand(OC_CALL, addr));

    // grab returned value, if any is expected
    if (c->lvalue != NULL) {
        struct asm_operand *ax = new_asm_operand_reg(REG_AX);
        ad.listing->ops->set_next_comment(ad.listing, "grab returned value");
        ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, lval, ax));
    }

    // clean up pushed arguments
    if (bytes_pushed > 0) {
        ad.listing->ops->set_next_comment(ad.listing, "clean up %d bytes that were pushed as arguments", bytes_pushed);
        ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_ADD, 
            new_asm_operand_reg(REG_SP), 
            new_asm_operand_imm(bytes_pushed)));
    }

    // caller restore registers (except AX)
}

static void code_conditional_jump(ir_entry *e) {
    // emit two things: compare, then appropriate jump.
    // conditions depend on whether the values are signed or unsigned,
    // good info here: https://www.cs.princeton.edu/courses/archive/spr18/cos217/lectures/14_Assembly2.pdf

    struct ir_entry_cond_jump_info *j = &e->t.conditional_jump;
    struct asm_operand *op1 = resolve_ir_value_to_asm_operand(j->v1);
    struct asm_operand *op2 = resolve_ir_value_to_asm_operand(j->v2);
    
    string *s = new_string();
    e->ops->to_string(e, s);
    ad.listing->ops->set_next_comment(ad.listing, "IR: %s", s->buffer);
    s->v->free(s);

    // this version does not support immediates in op1 (e.g. "if (1 == a)")
    if (op1->type == OT_IMMEDIATE) {
        error(NULL, 0, "comparison op1 cannot be an immediate");
        return;
    }

    if ((op1->type == OT_MEM_POINTED_BY_REG || op1->type == OT_MEM_OF_SYMBOL) &&
        (op2->type == OT_MEM_POINTED_BY_REG || op2->type == OT_MEM_OF_SYMBOL)) {
        // we cannot compare memory to memory (e.g. "(a > b)"), we must bring one into AX
        struct asm_operand *ax = new_asm_operand_reg(REG_AX);
        ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, ax, op1));
        ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_CMP, ax, op2));
    } else {
        ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_CMP, op1, op2));
    }

    // find the opcode, depending on the comparison flag
    enum opcode op;
    switch (j->cmp) {
        case IR_EQ: op = OC_JEQ; break;
        case IR_NE: op = OC_JNE; break;
        // let's use unsigned comparisons for now
        case IR_GT: op = OC_JAB; break;
        case IR_GE: op = OC_JAE; break;
        case IR_LT: op = OC_JBL; break;
        case IR_LE: op = OC_JBE; break;
    }

    struct asm_operand *addr = new_asm_operand_mem_by_sym(j->target_label);
    ad.listing->ops->add(ad.listing, new_asm_instruction_with_operand(op, addr));
}

static void code_unconditional_jump(ir_entry *e, char *label) {
    string *s = new_string();
    e->ops->to_string(e, s);
    ad.listing->ops->set_next_comment(ad.listing, "IR: %s", s->buffer);
    s->v->free(s);

    struct asm_operand *addr = new_asm_operand_mem_by_sym(label);
    ad.listing->ops->add(ad.listing, new_asm_instruction_with_operand(OC_JMP, addr));
}

static void code_return_statement(ir_entry *e) {
    struct ir_entry_return_info *info = &e->t.return_stmt;
    string *s = new_string();
    e->ops->to_string(e, s);
    ad.listing->ops->set_next_comment(ad.listing, "IR: %s", s->buffer);
    s->v->free(s);

    if (info->ret_val != NULL) {
        struct asm_operand *ax = new_asm_operand_reg(REG_AX);
        struct asm_operand *val = resolve_ir_value_to_asm_operand(info->ret_val);
        ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, ax,  val));
    }

    char target[128];
    target[sizeof(target) - 1] = 0;
    snprintf(target, sizeof(target) - 1, "%s_exit", ad.func_def->func_name);
    struct asm_operand *addr = new_asm_operand_mem_by_sym(target);
    ad.listing->ops->add(ad.listing, new_asm_instruction_with_operand(OC_JMP, addr));
}

static void code_simple_assignment(ir_entry *e, ir_value *lvalue, ir_value *rvalue) {
    string *s = new_string();
    e->ops->to_string(e, s);
    ad.listing->ops->set_next_comment(ad.listing, "IR: %s", s->buffer);
    s->v->free(s);

    struct asm_operand *lop = resolve_ir_value_to_asm_operand(lvalue);
    struct asm_operand *rop = resolve_ir_value_to_asm_operand(rvalue);

    if ((lop->type == OT_MEM_POINTED_BY_REG || lop->type == OT_MEM_OF_SYMBOL) &&
        (rop->type == OT_MEM_POINTED_BY_REG || rop->type == OT_MEM_OF_SYMBOL)) {
        // we cannot move memory to memory (e.g. "(a = b)"), we must bring one into AX
        struct asm_operand *ax = new_asm_operand_reg(REG_AX);
        ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, ax, rop));
        ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, lop, ax));
    } else {
        ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, lop, rop));
    }
}

static void code_unary_operation(ir_entry *e, ir_value *lvalue, ir_operation op, ir_value *rvalue) {
    // e.g. "a = ~b"
    // MOV AX, b
    // NOT/NEG AX
    // MOV a, AX

    string *s = new_string();
    e->ops->to_string(e, s);
    ad.listing->ops->set_next_comment(ad.listing, "IR: %s", s->buffer);
    s->v->free(s);

    struct asm_operand *ax = new_asm_operand_reg(REG_AX);
    struct asm_operand *lop = resolve_ir_value_to_asm_operand(lvalue);
    struct asm_operand *rop = resolve_ir_value_to_asm_operand(rvalue);

    switch (op) {
        case IR_NOT:
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, ax, rop));
            ad.listing->ops->add(ad.listing, new_asm_instruction_for_register(OC_NOT, REG_AX));
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, lop, ax));
            break;
        case IR_NEG:
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, ax, rop));
            ad.listing->ops->add(ad.listing, new_asm_instruction_for_register(OC_NEG, REG_AX));
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, lop, ax));
            break;
        case IR_ADDR_OF:
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_LEA, ax, rop));
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, lop, ax));
            break;
        case IR_VALUE_AT:
            // memory pointed by AX, without displacement.
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, ax, rop));
            struct asm_operand *ptr = new_asm_operand_mem_by_reg(REG_AX, 0);
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, lop, ptr));
            break;
        default:
            error(NULL, 0, "Unsupported IR unary operator %d", op);
            break;
    }
}

static void code_binary_operation(ir_entry *e, ir_value *lvalue, ir_value *rvalue1, ir_operation op, ir_value *rvalue2) {
    // e.g. "lval = rval1 + rval2"
    // MOV AX, rval1
    // ADD AX, rval2
    // MOV lval, AX

    string *s = new_string();
    e->ops->to_string(e, s);
    ad.listing->ops->set_next_comment(ad.listing, "IR: %s", s->buffer);
    s->v->free(s);
    
    struct asm_operand *ax = new_asm_operand_reg(REG_AX);
    struct asm_operand *cx = new_asm_operand_reg(REG_CX);
    struct asm_operand *dx = new_asm_operand_reg(REG_DX);
    struct asm_operand *lop = resolve_ir_value_to_asm_operand(lvalue);
    struct asm_operand *rop1 = resolve_ir_value_to_asm_operand(rvalue1);
    struct asm_operand *rop2 = resolve_ir_value_to_asm_operand(rvalue2);

    ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, ax, rop1));
    switch (op) {
        case IR_ADD:
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_ADD, ax, rop2));
            break;
        case IR_SUB:
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_SUB, ax, rop2));
            break;
        case IR_MUL:
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MUL, ax, rop2));
            break;
        case IR_DIV:
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_DIV, ax, rop2));
            break;
        case IR_MOD:
            // division puts remainder in DX, so move it to AX
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_DIV, ax, rop2));
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, ax, dx));
            break;
        case IR_AND:
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_AND, ax, rop2));
            break;
        case IR_OR:
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_OR, ax, rop2));
            break;
        case IR_XOR:
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_XOR, ax, rop2));
            break;
        case IR_LSH:
            // we must move num of bits to shift into CL
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, cx, rop2));
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_SHL, ax, cx));
            break;
        case IR_RSH:
            // we must move num of bits to shift into CL
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, cx, rop2));
            ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_SHR, ax, cx));
            break;
        default:
            error(NULL, 0, "Unsupported 3-code-addr binary operator %d", op);
            break;
    }

    ad.listing->ops->add(ad.listing, new_asm_instruction_with_operands(OC_MOV, lop, ax));
}

// when a temp register is no longer used, we release the storage it was allocated for it.
static void _release_temp_reg_allocations(ir_value *v, void *pdata, int idata) {
    if (v == NULL || v->type != IR_TREG)
        return;

    // most of this just for user friendly comments!
    ir_listing *ir = (ir_listing *)pdata;
    storage s;
    string *str;
    bool allocated;
    int curr_index = idata;
    int reg_no = v->val.temp_reg_no;
    int last_register_index = ir->ops->get_register_last_usage(ir, reg_no);
    if (curr_index >= last_register_index) {
        ad.allocator->ops->get_temp_reg_storage(ad.allocator, reg_no, &s, &allocated);
        str = new_string();
        ad.allocator->ops->storage_to_str(ad.allocator, &s, str);
        ad.allocator->ops->release_temp_reg_storage(ad.allocator, reg_no);
        ad.listing->ops->add_comment(ad.listing, "%s released from r%d", str->buffer, reg_no);
        str->v->free(str);
    }
}

static void assemble_function(ir_listing *ir, int start, int end) {
    // also see https://courses.cs.washington.edu/courses/cse401/06sp/codegen.pdf
    // we use asm_allocator to allocate storage space for temp registers
    // this storage space can be either CPU registers or stack space.
    // for each IR instruction, find appropriate assembly instruction(s)

    if (ir->entries_arr[start]->type != IR_FUNCTION_DEFINITION) {
        error(NULL, 0, "internal bug, function declaration IR was expected");
        return;
    }

    // house keeping first
    ad.func_def = &ir->entries_arr[start]->t.function_def;
    ad.allocator->ops->reset(ad.allocator);
    
    // declare stack variables and their offsets from BP:
    // this allows the allocator to grab more stack space as needed
    //   BP + n = last argument (that was pushed first, right-to-left)
    //   BP + 8 = first argument (that was pushed last of the arguments)
    //   BP + 4 = return address (that was pushed last)
    //   BP + 0 = previous BP
    //   BP - 4 = first local variable
    //   BP - n = local variable

    int bp_offset = options.pointer_size_bytes * 2; // skip pushed EBP and return address
    for (int i = 0; i < ad.func_def->args_len; i++) {
        ad.allocator->ops->declare_local_symbol(ad.allocator, 
            ad.func_def->args_arr[i].name, ad.func_def->args_arr[i].size,
            bp_offset);
        bp_offset += ad.func_def->args_arr[i].size;
    }

    bp_offset = 0; // to subtract the size of the first local variable, not of BP
    ad.stack_space_for_local_vars = 0;
    for (int i = start; i < end; i++) {
        ir_entry *e = ir->entries_arr[i];
        if (e->type == IR_DATA_DECLARATION && e->t.data_decl.storage == IR_LOCAL) {
            bp_offset -= e->t.data_decl.size; // note we subtract before
            ad.allocator->ops->declare_local_symbol(ad.allocator, 
                e->t.data_decl.symbol_name, e->t.data_decl.size,
                bp_offset);
            ad.stack_space_for_local_vars += e->t.data_decl.size;
        }
    }

    code_prologue();

    for (int i = start; i < end; i++) {
        ir_entry *e = ir->entries_arr[i];

        switch (e->type) {
            case IR_FUNCTION_DEFINITION:
                break;
            case IR_COMMENT:
                break;
            case IR_LABEL:
                ad.listing->ops->set_next_label(ad.listing, e->t.label.str);
                break;
            case IR_DATA_DECLARATION:
                // how about initial data?
                break;
            case IR_THREE_ADDR_CODE:
                struct ir_entry_three_addr_code_info *c = &e->t.three_address_code;
                if (c->lvalue != NULL && c->op1 == NULL && c->op == IR_NONE && c->op2 != NULL) {
                    // "lv = rv"
                    code_simple_assignment(e, c->lvalue, c->op2);
                } else if (c->lvalue != NULL && c->op1 == NULL && c->op != IR_NONE && c->op2 != NULL) {
                    // "lv = <unary> r2"
                    code_unary_operation(e, c->lvalue, c->op, c->op2);
                } else if (c->lvalue != NULL && c->op1 != NULL && c->op2 != NULL) {
                    // "lv = r1 <+> r2"
                    code_binary_operation(e, c->lvalue, c->op1, c->op, c->op2);
                } else {
                    // simple function calls might be encoded as "<ignored> = r1"
                    // it's the same as writing in C: "1;" or "a;", i.e. evaluation which result is ignored
                    printf("warning, unsupported 3-addr-code format: lv=%s, op1=%s, op2=%s", 
                        c->lvalue == NULL ? "null" : "non-null",
                        c->op1 == NULL    ? "null" : "non-null",
                        c->op2 == NULL    ? "null" : "non-null");
                }
                break;
            case IR_FUNCTION_CALL:
                code_function_call(e);
                break;
            case IR_CONDITIONAL_JUMP:
                code_conditional_jump(e);
                break;
            case IR_UNCONDITIONAL_JUMP:
                code_unconditional_jump(e, e->t.unconditional_jump.str);
                break;
            case IR_RETURN:
                code_return_statement(e);
                break;
        }

        // must free temp reg allocations, if this is the last entry they where used
        e->ops->foreach_ir_value(e, _release_temp_reg_allocations, ir, i);
    }

    code_epilogue();
}


// given an Intemediate Representation listing, generate an assembly listing.
void x86_assemble_ir_listing(ir_listing *ir_list, asm_listing *asm_list) {
    // prepare our things
    ad.allocator = new_asm_allocator(asm_list);
    ad.listing = asm_list;

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

void x86_encode_asm_into_machine_code(asm_listing *asm_list, enum x86_cpu_mode mode, obj_code *obj) {
    // encode this into intel machine code
    struct x86_encoder *enc = new_x86_encoder(mode, obj->text_seg, obj->relocations);
    asm_instruction *inst;
    struct encoding_info enc_info;
    encoded_instruction enc_inst;
    string *s;
    buffer *b;

    s = new_string();
    b = new_buffer();

    for (int i = 0; i < asm_list->length; i++) {
        inst = asm_list->instruction_ptrs[i];

        if (inst->label != NULL) {
            // we don't know if this is exported for now
            obj->symbols->add(obj->symbols, inst->label, obj->text_seg->length, SB_CODE);
        }

        if (inst->operation == OC_NONE)
            continue;
        
        if (!enc->encode_v4(enc, inst)) {
            string *s = new_string();
            asm_instruction_to_str(inst, s, false);
            error(NULL, 0, "Failed encoding instruction: '%s'\n", s->buffer);
            s->v->free(s);
            continue;
        }
    }

    printf("Sample resulting machine code\n");
    for (int i = 0; i < obj->text_seg->length; i++)
        printf("%02x ", (unsigned char)obj->text_seg->buffer[i]);
    printf("\n");
}

