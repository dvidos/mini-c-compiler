BINARY = mcc
SRC_FILES = \
	mcc.c \
	lexer.c \
	token.c \
	ast_node.c \
	ast.c \
	operators.c \
	parser/iterator.c \
	parser/recursive_descend.c \
	parser/shunting_yard.c

$(BINARY): $(SRC_FILES)
	gcc -o $@ -g $^

test: $(BINARY) sample.c
	./$(BINARY) sample.c

vtest: $(BINARY) sample.c
	./$(BINARY) -v sample.c

clean:
	rm -f mcc

