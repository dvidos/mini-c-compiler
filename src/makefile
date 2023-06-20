CFLAGS = -g -Werror
BINARY = mcc
RUNTIME_LIB = src/runtimes/libruntime.a
SRC_FILES = \
	mcc.c \
	err_handler.c \
	options.c \
	utils.c \
	utils/unit_tests.c \
	utils/buffer.c \
	utils/string.c \
	utils/list.c \
	utils/mempool.c \
	utils/data_types.c \
	utils/data_structs.c \
	compiler/lexer/token.c \
	compiler/lexer/token_list.c \
	compiler/lexer/lexer.c \
	compiler/statement.c \
	compiler/data_type.c \
	compiler/expression.c \
	compiler/operators.c \
	compiler/src_symbol.c \
	compiler/scope.c \
	compiler/declaration.c \
	compiler/ast.c \
	compiler/parser/iterator.c \
	compiler/parser/recursive_descend.c \
	compiler/parser/shunting_yard.c \
	compiler/analysis/analysis.c  \
	compiler/analysis/expr_analysis.c  \
	compiler/analysis/stmt_analysis.c \
	compiler/codegen/codegen.c \
	compiler/codegen/codegen_mod.c \
	compiler/codegen/codegen_stmt.c \
	compiler/codegen/codegen_expr.c \
	compiler/codegen/codegen_func.c \
	compiler/codegen/ir_value.c \
	compiler/codegen/ir_entry.c \
	compiler/codegen/ir_listing.c \
	assembler/encoder/encoder.c \
	assembler/encoder/asm_instruction.c \
	assembler/encoder/asm_listing.c \
	assembler/encoder/asm_allocator.c \
	assembler/encoder/encoded_instruction.c \
	assembler/encoder/encoding_info.c \
	assembler/assembler.c \
	assembler/asm_test.c \
	elf/elf64_contents.c \
	elf/ar.c \
	elf/obj_module.c \
	elf/write_elf.c \
	elf/elf_test.c \
	linker/symbol_table.c \
	linker/reloc_list.c \
	linker/section.c \
	linker/obj_code.c \
	linker/linker.c



target: $(RUNTIME_LIB) $(BINARY) # unit-tests integration-tests

unit-tests:
	./$(BINARY) --unit-tests

integration-tests:
	../run-tests.sh

$(BINARY): $(SRC_FILES)
	gcc -o $@ $(CFLAGS) $^

$(RUNTIME_LIB):
	cd runtimes && make


clean:
	rm -f mcc

