USERFILE=example
C_FLAGS=-fno-asynchronous-unwind-tables -nostdlib -nostdinc -fno-builtin


target:
	as --64 -o rt64.o rt64.asm
	gcc $(C_FLAGS) -c libc64_syscalls.c
	gcc $(C_FLAGS) -c libc64_misc.c
	gcc $(C_FLAGS) -c $(USERFILE).c
	ld -o $(USERFILE) $(USERFILE).o libc64_syscalls.o libc64_misc.o rt64.o
	objcopy -R .comment $(USERFILE)
	readelf -aW $(USERFILE) > $(USERFILE)_elf_contents

