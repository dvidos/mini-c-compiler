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


static void generate_failing_condition_jump(expression *condition, char *label, int label_num) {
    int r1, r2;
    oper op = condition->op;
    char *cmp_operation;

    // if exmpression op is comparison, generate appropriate code
    // if it is a symbol or boolean literal etc.
    
    if (op == OP_GE || op == OP_GT || 
        op == OP_LE || op == OP_LT || 
        op == OP_EQ || op == OP_NE)
    {
        expr_target *t1 = expr_target_temp_reg(cg.next_reg_num());
        expr_target *t2 = expr_target_temp_reg(cg.next_reg_num());
        generate_expression_code(condition->arg1, t1);
        generate_expression_code(condition->arg2, t2);
        ir.add_str("CMP t%d, t%d", r1, r2);
        switch (op) {
            case OP_GE: cmp_operation = "JLT"; break;
            case OP_GT: cmp_operation = "JLE"; break;
            case OP_LE: cmp_operation = "JGT"; break;
            case OP_LT: cmp_operation = "JGE"; break;
            case OP_EQ: cmp_operation = "JNE"; break;
            case OP_NE: cmp_operation = "JEQ"; break;
        }
    } else {
        expr_target *t1 = expr_target_temp_reg(cg.next_reg_num());
        generate_expression_code(condition, t1);
        ir.add_str("CMP t%d, 0", r1, r2);
        cmp_operation = "JEQ"; // jump if EQ to zero, i.e. to false.
    }
    
    char formatted_label[32];
    sprintf(formatted_label, label, label_num);
    ir.add_str("%s %s", cmp_operation, formatted_label); 
}

void generate_statement_code(statement *stmt) {

    // blocks of statements and expressions, together with jumps
    switch (stmt->stmt_type) {
        case ST_BLOCK:
            // don't we need to push scope? in order
            // to make sure inner names resolve to correct target?
            statement *entry = stmt->body;
            while (entry != NULL) {
                generate_statement_code(entry);
                entry = entry->next;
            }
            break;

        case ST_VAR_DECL:
            if (cg.curr_func_name() == NULL) {
                // allocate space in data segment
                ir.add_str(".data \"%s\" %d bytes", stmt->decl->var_name, stmt->decl->data_type->ops->size_of(stmt->decl->data_type));
            } else {
                // stack allocation has already happened, set initial value
                if (stmt->expr != NULL) {
                    int offset = cg.get_local_var_bp_offset(stmt->decl->var_name);
                    expr_target *target = expr_target_stack_location(offset);
                    generate_expression_code(stmt->expr, target);
                }
            }
            break;

        case ST_IF:
            // generate condition and jumps in the true and false bodies
            int ifno = cg.next_if_num();
            if (stmt->else_body == NULL) {
                // simple if
                generate_failing_condition_jump(stmt->expr, "if%d_end", ifno);
                generate_statement_code(stmt->body);
                ir.set_next_label("if%d_end", ifno);
            } else {
                // if & else
                generate_failing_condition_jump(stmt->expr, "if%d_false", ifno);
                generate_statement_code(stmt->body);
                ir.jmp("if%d_end", ifno);
                ir.set_next_label("if%d_false", ifno);
                generate_statement_code(stmt->else_body);
                ir.set_next_label("if%d_end", ifno);
            }
            break;

        case ST_WHILE:
            // generate condition and jumps in the end of the loop
            // we need a stack of whiles
            cg.push_while();
            ir.set_next_label("wh%d", cg.curr_while_num());
            generate_failing_condition_jump(stmt->expr, "wh%d_end", cg.curr_while_num());
            generate_statement_code(stmt->body);
            ir.jmp("wh%d", cg.curr_while_num());
            ir.set_next_label("wh%d_end", cg.curr_while_num());
            cg.pop_while();
            break;

        case ST_CONTINUE:
            if (cg.curr_while_num() == 0)
                error(stmt->token->filename, stmt->token->line_no, "continue without while");
            else
                ir.add_str("JMP wh%d", cg.curr_while_num());
            break;

        case ST_BREAK:
            if (cg.curr_while_num() == 0)
                error(stmt->token->filename, stmt->token->line_no, "break without while");
            else
                ir.add_str("JMP wh%d_end", cg.curr_while_num());
            break;

        case ST_RETURN:
            if (stmt->expr != NULL)
                generate_expression_code(stmt->expr, expr_target_return_register());
            ir.jmp("%s_exit", cg.curr_func_name());
            break;

        case ST_EXPRESSION:
            // there may be expressions that don't return anything, e.g. calling void functions.
            generate_expression_code(stmt->expr, expr_target_return_register());
            break;    
    }
}


