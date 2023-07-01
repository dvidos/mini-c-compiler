USERFILE=example
LIBRARY64=runtime64
C_FLAGS=-std=c99 -fno-asynchronous-unwind-tables -nostdlib -nostdinc -fno-builtin


target64:
	as --64 -o rt64.o rt64.asm
	gcc $(C_FLAGS) -c libc64_syscalls.c
	gcc $(C_FLAGS) -c libc64_misc.c
	rm -f lib$(LIBRARY64).a
	ar rv lib$(LIBRARY64).a rt64.o libc64_syscalls.o libc64_misc.o
	# gcc $(C_FLAGS) -c $(USERFILE).c
	# ld -o $(USERFILE) $(USERFILE).o -L. -l$(LIBRARY64)
	# readelf -aW $(USERFILE) > $(USERFILE)_elf_contents

clean:
	rm *.o $(USERFILE)
