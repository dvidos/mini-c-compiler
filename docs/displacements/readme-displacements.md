
Trying to see how jumps and calls displacements work.

Apparently they are relative (positive and negative), and also relative to the end of the instruction, not to the start of it.

Created via:

`nasm -f elf32 displacements.asm && objdump -d displacements.o > displacements.dis`
