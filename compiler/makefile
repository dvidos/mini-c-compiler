BINARY = mcc
SRC_FILES = mcc.c parser.c atom.c token.c

$(BINARY): $(SRC_FILES)
	gcc -o $@ -g $^

test: $(BINARY) test.c
	./$(BINARY) test.c

clean:
	rm -f mcc

