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
#include "../src_symbol.h"
#include "../declaration.h"
#include "codegen.h"


// we generate for the false condition, to allow to skip an "if"s body.
static void gen_false_cond_jump(code_gen *cg, expression *expr, char *label_fmt, int label_num) {

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
        v1 = cg->ops->create_ir_value(cg, expr->arg1);
        v2 = cg->ops->create_ir_value(cg, expr->arg2);
    } else {
        // evaluate expression in a boolean (non-zero) context
        v1 = cg->ops->create_ir_value(cg, expr);
        v2 = new_ir_value_immediate(0);
        cmp = IR_EQ;
    }

    cg->ir->ops->add(cg->ir, new_ir_conditional_jump(v1, cmp, v2, label_fmt, label_num));
}

void code_gen_generate_for_statement(code_gen *cg, statement *stmt) {
    int num;

    // blocks of statements and expressions, together with jumps
    switch (stmt->stmt_type) {
        case ST_BLOCK:
            // don't we need to push scope? in order
            // to make sure inner names resolve to correct target?
            statement *s = stmt->body;
            while (s != NULL) {
                code_gen_generate_for_statement(cg, s);
                s = s->next;
            }
            break;

        case ST_VAR_DECL:
            // global vars are done, we only care about init value of locals
            // ideally, we should have one variable for each scope but...
            if (cg->ops->get_curr_func_name(cg) == NULL) {
                error(stmt->token->filename, stmt->token->line_no, "function name not found, when generating var decl for statement");
                return;
            }
            if (stmt->expr != NULL)
                cg->ops->generate_for_expression(cg, new_ir_value_symbol(stmt->decl->var_name), stmt->expr);
            break;

        case ST_IF:
            num = cg->ops->next_label_num(cg);
            if (stmt->else_body == NULL) {
                // simple if, one jump at end
                gen_false_cond_jump(cg, stmt->expr, "if_%d_end", num);
                cg->ops->generate_for_statement(cg, stmt->body);
                cg->ir->ops->add(cg->ir, new_ir_label("if_%d_end", num));
            } else {
                // if & else bodies, jump to false, skip false
                gen_false_cond_jump(cg, stmt->expr, "if_%d_false", num);
                cg->ops->generate_for_statement(cg, stmt->body);
                cg->ir->ops->add(cg->ir, new_ir_unconditional_jump("if_%d_end", num));
                cg->ir->ops->add(cg->ir, new_ir_label("if_%d_false", num));
                cg->ops->generate_for_statement(cg, stmt->else_body);
                cg->ir->ops->add(cg->ir, new_ir_label("if_%d_end", num));
            }
            break;

        case ST_WHILE:
            cg->ops->begin_loop_generation(cg);
            num = cg->ops->curr_loop_num(cg);
            cg->ir->ops->add(cg->ir, new_ir_label("while_%d_begin", num));
            gen_false_cond_jump(cg, stmt->expr, "while_%d_end", num);
            cg->ops->generate_for_statement(cg, stmt->body);
            cg->ir->ops->add(cg->ir, new_ir_unconditional_jump("while_%d_begin", num));
            cg->ir->ops->add(cg->ir, new_ir_label("while_%d_end", num));
            cg->ops->end_loop_generation(cg);
            break;

        case ST_CONTINUE:
            num = cg->ops->curr_loop_num(cg);
            if (num == 0) {
                error(stmt->token->filename, stmt->token->line_no, "break without while");
                return;
            }
            cg->ir->ops->add(cg->ir, new_ir_unconditional_jump("while_%d_begin", num));
            break;

        case ST_BREAK:
            num = cg->ops->curr_loop_num(cg);
            if (num == 0) {
                error(stmt->token->filename, stmt->token->line_no, "break without while");
                return;
            }
            cg->ir->ops->add(cg->ir, new_ir_unconditional_jump("while_%d_end", num));
            break;

        case ST_RETURN:
            if (cg->ops->get_curr_func_name(cg) == NULL) {
                error(stmt->token->filename, stmt->token->line_no, "return without a function context");
                return;
            }
            if (stmt->expr != NULL)
                cg->ops->generate_for_expression(cg, new_ir_value_symbol("ret_val"), stmt->expr);
            cg->ir->ops->add(cg->ir, new_ir_unconditional_jump("%s_end", cg->ops->get_curr_func_name(cg)));
            break;

        case ST_EXPRESSION:
            // there may be expressions that don't return anything, e.g. calling void functions.
            cg->ops->generate_for_expression(cg, new_ir_value_symbol("(ignored)"), stmt->expr);
            break;    
    }
}

