SRC=hello.asm
OBJ=$(SRC:.asm=.o)
BIN=$(SRC:.asm=)

$(BIN): $(SRC)
	nasm $< -o $(OBJ) -f elf32
	ld  $(OBJ) -o $@ -m elf_i386
	strip $@

