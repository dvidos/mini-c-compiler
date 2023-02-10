#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../options.h"
#include "../err_handler.h"
#include "../expression.h"
#include "../lexer/token.h"
#include "../statement.h"
#include "../symbol.h"
#include "../declaration.h"
#include "codegen.h"
#include "interm_repr.h"


/*
    For assembly opcodes one can see Intel's books
    https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
    
    Or this useful website:
    http://ref.x86asm.net/coder32.html


*/


static int next_reg_num();
static int next_var_num();
static int next_if_num();
static int curr_while_num();
static void push_while();
static void pop_while();

static void assign_curr_func(func_declaration *func);
static char *curr_func_name();
static void register_local_var(var_declaration *decl, bool is_arg, int arg_no);
static int get_local_var_bp_offset(char *name);

// codegen_stmt.c
void generate_statement_code(statement *stmt);

// codegen_expr.c
void generate_expression_code(expression *expr, int target_reg_no, char *target_symbol_name);


code_gen cg = {
    .next_reg_num = next_reg_num,
    .next_var_num = next_var_num,
    .next_if_num = next_if_num,
    .curr_while_num = curr_while_num,
    .push_while = push_while,
    .pop_while = pop_while,

    .assign_curr_func = assign_curr_func,
    .curr_func_name = curr_func_name,
    .register_local_var = register_local_var,
    .get_local_var_bp_offset = get_local_var_bp_offset,

    .generate_expression_code = generate_expression_code,
    .generate_statement_code = generate_statement_code,
};

// --------------------------------------------------------------------

#define WHILES_STACK_SIZE  16

static int regs_counter = 1; 
static int vars_counter = 0; 
static int ifs_counter = 0;
static int whiles_counter = 0;
static int whiles_stack_len;
static int whiles_stack[WHILES_STACK_SIZE];

static int next_reg_num() {
    return ++regs_counter;
}

static int next_var_num() {
    return ++vars_counter;
}

static int next_if_num() {
    return ++ifs_counter;
}

static int curr_while_num() {
    if (whiles_stack_len == 0) {
        error(NULL, 0, "while was expected, but none pushed");
        return 0;
    }
    return whiles_stack[whiles_stack_len - 1];
}

static void push_while() {
    if (whiles_stack_len == WHILES_STACK_SIZE) {
        error(NULL, 0, "more than %d nested whiles detected, we need more stack!", WHILES_STACK_SIZE);
        return;
    }
    whiles_stack[whiles_stack_len++] = ++whiles_counter;
}

static void pop_while() {
    if (whiles_stack_len == 0) {
        error(NULL, 0, "stack of whiles underflow (that shouldn't happen)");
        return;
    }
    whiles_stack_len--;
}

// ---------------------------------------------------

struct local_variable_info {
    var_declaration *decl;
    int bp_offset;
};

#define LOCAL_VARS_ARR_SIZE  32
static func_declaration *curr_func;
static struct local_variable_info curr_func_vars[LOCAL_VARS_ARR_SIZE];
static int curr_func_vars_count;

static void assign_curr_func(func_declaration *func) {
    curr_func = func;
    memset(curr_func_vars, 0, sizeof(curr_func_vars));
    curr_func_vars_count = 0;
}

static char *curr_func_name() {
    return curr_func == NULL ? NULL : curr_func->func_name;
}

static void register_local_var(var_declaration *decl, bool is_arg, int arg_no) {
    // assume ABI - i.e. BP used as frame pointer
    // find double declaration, forbid for now
    // deduce offset automatically
    // error if no space for other local vars

    // function args have been pushed before BP, therefore have posibive offset
    // locally allocate variables are allocated below BP, therefore have negative offset

    // https://en.wikibooks.org/wiki/X86_Disassembly/Functions_and_Stack_Frames
    /*  :    : 
        |  2 | [ebp + 16] (3rd function argument)
        |  5 | [ebp + 12] (2nd argument)
        | 10 | [ebp + 8]  (1st argument)
        | RA | [ebp + 4]  (return address)
        | FP | [ebp]      (old ebp value)
        |    | [ebp - 4]  (1st local variable)
        :    :
        :    :
        |    | [ebp - X]  (esp - the current stack pointer. The use of push / pop is valid now) */


    if (curr_func_vars_count >= LOCAL_VARS_ARR_SIZE) {
        error(decl->token->filename, decl->token->line_no, "local vars array filled, need more room!");
        return;
    }

    int last_offset = 0;
    if (curr_func_vars_count > 0)
        last_offset = curr_func_vars[curr_func_vars_count - 1].bp_offset;
    int var_size = decl->data_type->ops->size_of(decl->data_type);

    curr_func_vars[curr_func_vars_count].decl = decl;
    curr_func_vars[curr_func_vars_count].bp_offset += last_offset + var_size;
    curr_func_vars_count++;
}

static int get_local_var_bp_offset(char *name) {
    for (int i = 0; i < curr_func_vars_count; i++) {
        if (strcmp(curr_func_vars[i].decl->var_name, name) == 0)
            return curr_func_vars[i].bp_offset;
    }
    return 0; // not found
}

// ----------------------------------------------

static void generate_global_variable_code(var_declaration *decl, expression *init_expr) {
    // the initial value, has to be on data segment or where?
    void *init_value = NULL;
    if (init_expr != NULL) {
        if (init_expr->op == OP_STR_LITERAL) {
            // init_value = ir.get_strz_offset(init_expr->value.str, init_expr->token);
            init_value = init_expr->value.str;
        }
        else if (init_expr->op == OP_NUM_LITERAL) {
            // must make sure of the size here...
            init_value = &init_expr->value.num;
        }
        else if (init_expr->op == OP_CHR_LITERAL) {
            // must make sure of the size here...
            init_value = &init_expr->value.chr;
        }
        else if (init_expr->op == OP_BOOL_LITERAL) {
            // must make sure of the size here...
            init_value = &init_expr->value.bln;
        }
        else {
            error(init_expr->token->filename, init_expr->token->line_no,
                "sorry, only literal initial values supported for now");
            return;
        }
    }

    // allocate memory, give size, get address
    int offset = ir.reserve_data(
        decl->data_type->ops->size_of(decl->data_type),
        init_value
    );
    ir.add_symbol(decl->var_name, false, offset);
}

static void generate_function_code(func_declaration *func) {
    assign_curr_func(func);

    // stack frame, decoding of arguments, local data, etc.
    // where do we go from here?

    ir.set_next_label("%s", func->func_name);
    char reg_prefix = options.is_64_bits ? 'R' : 'E';
    ir.add_str("PUSH %cBP", reg_prefix);
    ir.add_str("MOV %cBP, %cSP", reg_prefix, reg_prefix);

    var_declaration *arg = func->args_list;
    int arg_no = 0;
    while (arg != NULL) {
        cg.register_local_var(arg, true, arg_no);
        arg_no ++;
        arg = arg->next;
    }

    // we also need to understand all the locals we need to allocate, then do:
    ir.add_str("SUB %cSP, <bytes for local allocation>", reg_prefix); // or whatever bytes are needed for local variables

    statement *stmt = func->stmts_list;
    while (stmt != NULL) {
        generate_statement_code(stmt);
        stmt = stmt->next;
    }

    ir.set_next_label("%s_exit", func->func_name);
    ir.add_str("MOV %cSP, %cBP", reg_prefix, reg_prefix);
    ir.add_str("POP %cBP", reg_prefix);
    ir.add_str("RET");
}

void generate_module_code(ast_module_node *module) {

    statement *stmt = module->statements_list;
    while (stmt != NULL) {
        if (stmt->stmt_type != ST_VAR_DECL) {
            error(stmt->token->filename, stmt->token->line_no, "only var declarations are supported in code generation");
        } else {
            generate_global_variable_code(stmt->decl, stmt->expr);
        }
        stmt = stmt->next;
    }


    func_declaration *func = module->funcs_list;
    while (func != NULL) {
        if (func->stmts_list == NULL) { // it's just a declaration
            func = func->next;
            continue;
        }

        ir.add_comment("; --- function %s() ----------------------", func->func_name);
        generate_function_code(func);
        ir.add_comment("");

        func = func->next;
    }
}

