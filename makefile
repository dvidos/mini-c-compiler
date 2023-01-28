BINARY = mcc
SRC_FILES = \
	mcc.c \
	error.c \
	options.c \
	lexer/token.c \
	lexer/lexer.c \
	ast_node.c \
	data_type.c \
	expression.c \
	ast.c \
	operators.c \
	parser/iterator.c \
	parser/recursive_descend.c \
	parser/shunting_yard.c \
	symbol.c \
	scope.c \
	analysis.c


# target: tested
target: $(BINARY)


tested: $(BINARY) run-tests.sh
	./run-tests.sh

$(BINARY): $(SRC_FILES)
	gcc -o $@ -g $^

clean:
	rm -f mcc

