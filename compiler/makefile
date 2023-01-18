BINARY = mcc
SRC_FILES = \
	mcc.c \
	atom.c \
	lexer.c \
	token.c \
	ast.c \
	parser.c \

$(BINARY): $(SRC_FILES)
	gcc -o $@ -g $^

test: $(BINARY) test.c
	./$(BINARY) test.c

vtest: $(BINARY) test.c
	./$(BINARY) -v test.c

clean:
	rm -f mcc

