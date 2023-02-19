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

static void assign_curr_func(func_declaration *decl);
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

struct local_var_info {
    var_declaration *decl;
    bool is_arg;
    int arg_no;
    int bp_offset;
    int size_bytes;
};

struct curr_func_info {
    func_declaration *decl;

    struct local_var_info *vars;
    int vars_count;
    int vars_capacity;
};

static struct curr_func_info *curr_func = NULL;


static void assign_curr_func(func_declaration *decl) {
    if (curr_func == NULL) {
        curr_func = malloc(sizeof(struct curr_func_info));
        memset(curr_func, 0, sizeof(struct curr_func_info));

        curr_func->vars_capacity = 5;
        curr_func->vars = malloc(sizeof(struct local_var_info) * curr_func->vars_capacity);
    }

    curr_func->decl = decl;
    curr_func->vars_count = 0;
    memset(curr_func->vars, 0, sizeof(curr_func->vars_capacity * sizeof(struct local_var_info)));
}

static char *curr_func_name() {
    return curr_func == NULL || curr_func->decl == NULL ? 
        NULL : 
        curr_func->decl->func_name;
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

    if (curr_func == NULL || curr_func->decl == NULL) {
        error(decl->token->filename, decl->token->line_no, "current function not registered yet!");
        return;
    }

    // extend storage if needed
    if (curr_func->vars_count + 1 >= curr_func->vars_capacity) {
        curr_func->vars_capacity *= 2;
        curr_func->vars = realloc(curr_func->vars, curr_func->vars_capacity * sizeof(struct local_var_info));
    }   

    // find size to allocate
    int size = decl->data_type->ops->size_of(decl->data_type);

    // find BP offset
    int bp_offset;
    if (is_arg) {
        // need to skip two pushed values: old BP value & return address
        // i.e. 1st arg at [EBP + 8], 2nd at [EBP + 12], 3rd at [EBP + 16]
        bp_offset = (arg_no + 2) * options.pointer_size;
    } else {
        // first local variable is below EBP
        bp_offset = -size;
        // but subsequent are even lower
        if (curr_func->vars_count > 0 && curr_func->vars[curr_func->vars_count - 1].bp_offset < 0) {
            bp_offset = curr_func->vars[curr_func->vars_count - 1].bp_offset - size;
        }
    }

    curr_func->vars[curr_func->vars_count].decl = decl;
    curr_func->vars[curr_func->vars_count].is_arg = is_arg;
    curr_func->vars[curr_func->vars_count].arg_no = arg_no;
    curr_func->vars[curr_func->vars_count].bp_offset = bp_offset;
    curr_func->vars[curr_func->vars_count].size_bytes = size;
    curr_func->vars_count++;
}

static int get_local_var_bp_offset(char *name) {

    if (curr_func == NULL || curr_func->decl == NULL)
        return 0; // invalid

    for (int i = 0; i < curr_func->vars_count; i++) {
        if (strcmp(curr_func->vars[i].decl->var_name, name) == 0)
            return curr_func->vars[i].bp_offset;
    }

    return 0; // not found - zero offset is invalid
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


static void gather_local_var_declarations(statement *stmt) {
    if (stmt == NULL)
        return;
    
    switch (stmt->stmt_type) {
        case ST_VAR_DECL:
            // bingo!
            cg.register_local_var(stmt->decl, false, 0);
            break;

        case ST_BLOCK:
            statement *inner = stmt->body;
            while (inner != NULL) {
                gather_local_var_declarations(inner);
                inner = inner->next;
            }
            break;

        case ST_IF:
            gather_local_var_declarations(stmt->body);
            gather_local_var_declarations(stmt->else_body);
            break;

        case ST_WHILE:
            gather_local_var_declarations(stmt->body);
            break;

        case ST_CONTINUE: // fallthrough
        case ST_BREAK:
        case ST_RETURN:
        case ST_EXPRESSION:
            break;    
    }
}

void generate_local_var_code() {
    if (curr_func->vars_count == 0)
        return;

    for (int i = 0; i < curr_func->vars_count; i++) {
        struct local_var_info *var = &curr_func->vars[i];
        if (var->is_arg) {
            ir.add_comment("\t; arg #%d, %s %s, located at %cBP+%d", 
                var->arg_no,
                var->decl->data_type->ops->to_string(var->decl->data_type), 
                var->decl->var_name, 
                options.register_prefix,
                var->bp_offset);
        } else {
            ir.add_str("SUB %cSP, %d   ; %s %s, at %cBP%d", 
                options.register_prefix, 
                var->size_bytes,
                var->decl->data_type->ops->to_string(var->decl->data_type), 
                var->decl->var_name,
                options.register_prefix,
                var->bp_offset
            );
        }
    }
}

static void generate_function_code(func_declaration *func) {

    assign_curr_func(func);

    // register arguments as local variables
    int arg_no = 0;
    var_declaration *arg = func->args_list;
    while (arg != NULL) {
        cg.register_local_var(arg, true, arg_no++);
        arg = arg->next;
    }

    // traverse function tree to find local variables.
    statement *stmt = func->stmts_list;
    while (stmt != NULL) {
        gather_local_var_declarations(stmt);
        stmt = stmt->next;
    }

    // establish stack frame
    ir.set_next_label("%s", func->func_name);
    ir.add_str("PUSH %cBP", options.register_prefix);
    ir.add_str("MOV %cBP, %cSP", options.register_prefix, options.register_prefix);

    // generate code for stack allocation
    generate_local_var_code();

    // generate code for function statements tree
    stmt = func->stmts_list;
     while (stmt != NULL) {
        generate_statement_code(stmt);
        stmt = stmt->next;
    }

    // preparng to exit
    ir.set_next_label("%s_exit", func->func_name);
    ir.add_str("MOV %cSP, %cBP", options.register_prefix, options.register_prefix);
    ir.add_str("POP %cBP", options.register_prefix);
    ir.add_str("RET");
    ir.add_str("");
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

