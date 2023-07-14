CFLAGS = -g -Werror
BINARY = mcc
RUNTIME_LIB = runtimes/libruntime64.a
SRC_FILES = \
	mcc.c \
	err_handler.c \
	run_info.c \
	utils.c \
	utils/unit_tests.c \
	utils/mempool.c \
	$(wildcard utils/data_types/*.c) \
	$(wildcard utils/data_structs/*.c) \
	compiler/lexer/token.c \
	compiler/lexer/lexer.c \
	$(wildcard compiler/ast/*.c) \
	compiler/scoped_symbol.c \
	compiler/scope.c \
	compiler/parser/parser.c \
	compiler/parser/token_iterator.c \
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
	assembler/encoder/asm_allocator.c \
	assembler/encoder/encoded_instruction.c \
	assembler/encoder/encoding_info.c \
	assembler/ir_to_asm_converter.c \
	assembler/asm_line.c \
	assembler/asm_listing.c \
	assembler/assembler.c \
	assembler/asm_test.c \
	elf/elf64_contents.c \
	elf/ar.c \
	elf/obj_module.c \
	elf/obj_section.c \
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

