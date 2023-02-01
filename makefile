BINARY = mcc
SRC_FILES = \
	mcc.c \
	err_handler.c \
	options.c \
	lexer/token.c \
	lexer/lexer.c \
	statement.c \
	data_type.c \
	expression.c \
	operators.c \
	symbol.c \
	scope.c \
	ast_node.c \
	ast.c \
	parser/iterator.c \
	parser/recursive_descend.c \
	parser/shunting_yard.c \
	analysis/analysis.c  \
	analysis/expr_analysis.c  \
	analysis/stmt_analysis.c \
	codegen/codegen.c \
	codegen/interm_repr.c


# target: tested
target: $(BINARY)


tested: $(BINARY) run-tests.sh
	./run-tests.sh

$(BINARY): $(SRC_FILES)
	gcc -o $@ -g $^

clean:
	rm -f mcc

