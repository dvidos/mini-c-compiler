CFLAGS = -g -Werror
BINARY = mcc
SRC_FILES = \
	mcc.c \
	err_handler.c \
	options.c \
	utils.c \
	utils/buffer.c \
	lexer/token.c \
	lexer/token_list.c \
	lexer/lexer.c \
	statement.c \
	data_type.c \
	expression.c \
	operators.c \
	src_symbol.c \
	scope.c \
	declaration.c \
	ast.c \
	parser/iterator.c \
	parser/recursive_descend.c \
	parser/shunting_yard.c \
	analysis/analysis.c  \
	analysis/expr_analysis.c  \
	analysis/stmt_analysis.c \
	codegen/codegen.c \
	codegen/codegen_mod.c \
	codegen/codegen_stmt.c \
	codegen/codegen_expr.c \
	codegen/codegen_func.c \
	codegen/ir_value.c \
	codegen/ir_entry.c \
	codegen/ir_listing.c \
	elf/read_elf.c \
	elf/write_elf.c \
	elf/elf_test.c \
	binary/binary_gen.c \
	binary/parse_asm.c \
	binary/encode_asm.c \
	x86_encoder/symbol_table.c \
	x86_encoder/reloc_list.c \
	x86_encoder/instruction.c \
	x86_encoder/encoder.c \
	x86_encoder/asm_test.c \
	x86_encoder/asm_listing.c \
	x86_encoder/obj_code.c \
	x86_encoder/assembler.c \
	x86_encoder/linker.c




# target: tested
target: $(BINARY)


tested: $(BINARY) run-tests.sh
	./run-tests.sh

$(BINARY): $(SRC_FILES)
	gcc -o $@ $(CFLAGS) $^

clean:
	rm -f mcc

