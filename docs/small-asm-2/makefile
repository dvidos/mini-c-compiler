SRC=hello.asm
OBJ=$(SRC:.asm=.o)
BIN=$(SRC:.asm=)

$(BIN): $(SRC)
	nasm $< -o $(OBJ) -f elf32
	ld  $(OBJ) -o $@ -m elf_i386  # -n turns of paging and puts everything into one load program ,still .text is 1024 long in the file
	strip $@

