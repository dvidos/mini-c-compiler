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
#include "interm_repr.h"
#include "codegen.h"


static void generate_conditional_jump_for_false(ir_listing *listing, expression *expr, char *label_fmt, int label_num) {

    ir_value *v1;
    ir_comparison cmp = IR_NONE;
    ir_value *v2;
    oper op = expr->op;

    if (op == OP_GE || op == OP_GT || op == OP_LE || op == OP_LT || op == OP_EQ || op == OP_NE) {
        switch (expr->op) {
            case OP_EQ: cmp = IR_NE; break;
            case OP_NE: cmp = IR_EQ; break;
            case OP_GT: cmp = IR_LE; break;
            case OP_GE: cmp = IR_LT; break;
            case OP_LT: cmp = IR_GE; break;
            case OP_LE: cmp = IR_GT; break;
        }
        v1 = new_ir_value_register(cg.next_reg_num());
        generate_expression_ir_code(listing, v1, expr->arg1);
        v2 = new_ir_value_register(cg.next_reg_num());
        generate_expression_ir_code(listing, v2, expr->arg1);
    } else {
        cmp = IR_EQ;
        v1 = new_ir_value_register(cg.next_reg_num());
        v2 = new_ir_value_immediate(0);
        generate_expression_ir_code(listing, v1, expr);
    }

    listing->ops->add(listing, new_ir_conditional_jump(v1, cmp, v2, label_fmt, label_num));
}

void generate_statement_ir_code(ir_listing *listing, statement *stmt) {
    ir_value *lv;

    // blocks of statements and expressions, together with jumps
    switch (stmt->stmt_type) {
        case ST_BLOCK:
            // don't we need to push scope? in order
            // to make sure inner names resolve to correct target?
            statement *s = stmt->body;
            while (s != NULL) {
                generate_statement_ir_code(listing, s);
                s = s->next;
            }
            break;

        case ST_VAR_DECL:
            // global vars are done, we only care about init value of locals
            // ideally, we should have one variable for each scope but...
            if (cg.curr_func_name() != NULL && stmt->expr != NULL) {
                generate_expression_ir_code(listing, new_ir_value_symbol(stmt->decl->var_name), stmt->expr);
            }
            break;

        case ST_IF:
            // generate condition and jumps in the true and false bodies
            int num_if = cg.next_if_num();
            // we need to generate the code of the expression.
            // no matter if in comparative format, or in boolean format.
            if (stmt->else_body == NULL) {
                // simple if, one jump at end
                generate_conditional_jump_for_false(listing, stmt->expr, "if%d_end", num_if);
                generate_statement_ir_code(listing, stmt->body);
                listing->ops->add(listing, new_ir_label("if%d_end", num_if));
            } else {
                // if & else bodies
                generate_conditional_jump_for_false(listing, stmt->expr, "if%d_false", num_if);
                generate_statement_ir_code(listing, stmt->body);
                listing->ops->add(listing, new_ir_unconditional_jump("if%d_end", num_if));
                listing->ops->add(listing, new_ir_label("if%d_false", num_if));
                generate_statement_ir_code(listing, stmt->else_body);
                listing->ops->add(listing, new_ir_label("if%d_end", num_if));
            }
            break;

        case ST_WHILE:
            // generate condition and jumps in the end of the loop
            // we need a stack of whiles
            cg.push_while();
            listing->ops->add(listing, new_ir_label("while%d_start", cg.curr_while_num()));
            generate_conditional_jump_for_false(listing, stmt->expr, "while%d_end", num_if);
            generate_statement_ir_code(listing, stmt->body);
            listing->ops->add(listing, new_ir_unconditional_jump("while%d_start", cg.curr_while_num()));
            listing->ops->add(listing, new_ir_label("while%d_end", cg.curr_while_num()));
            cg.pop_while();
            break;

        case ST_CONTINUE:
            if (cg.curr_while_num() == 0) {
                error(stmt->token->filename, stmt->token->line_no, "continue without while");
                return;
            }
            listing->ops->add(listing, new_ir_unconditional_jump("while%d_start", cg.curr_while_num()));
            break;

        case ST_BREAK:
            if (cg.curr_while_num() == 0) {
                error(stmt->token->filename, stmt->token->line_no, "break without while");
                return;
            }
            listing->ops->add(listing, new_ir_unconditional_jump("while%d_end", cg.curr_while_num()));
            break;

        case ST_RETURN:
            // generate to the "return_value" local symbol
            generate_expression_ir_code(listing, new_ir_value_symbol("ret_val"), stmt->expr);
            listing->ops->add(listing, new_ir_unconditional_jump("%s_exit", cg.curr_func_name));
            break;

        case ST_EXPRESSION:
            // there may be expressions that don't return anything, e.g. calling void functions.
            generate_expression_ir_code(listing, new_ir_value_symbol("/dev/null"), stmt->expr);
            break;    
    }
}


