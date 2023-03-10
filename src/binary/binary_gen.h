#pragma once
#include "../elf/elf_contents.h"

/*

I tried going the other way: how to produce the smallest
executable I could.

https://www.linuxjournal.com/article/1060

ELF file format is widely documented, so i'll repeat it here for my understanding!
ELF file format supports executables, object files (.o) and shared libraries (.so)!
The type of file (e.g. DYN or EXEC) is available in the header.

It is mainly divided in sections, e.g. one for .text, one for .data and one for .bss.
Each section can be marked writable or read only. Another section may have a symbol table.
We also need to include any relocations e.g. references to external libraries.

* One file header up top.
* An area (table) with zero or more program headers, each with information about a segment.
  (the main header has information about the offset, the number of entries and their size)
* An area (table) with zero or more section headers, each with information about a segment.
  (the main header has information about the offset, the number of entries and their size)
* ...

Oh well, it seems there's too much trouble, one has to set things up 
to work for the target system they intend to run on. For example, 
there is even the name of the dynamic loader to use (/lib64/ld-linux-x86-64.so.2
in my case) in the file. It seems too much trouble for just a simple "Hello world" call.

Well, the problem is that we are still incurring a lot of overhead by using the main() 
interface. The linker is still adding an interface to the OS for us, and it is that 
interface that actually calls main(). 

https://www.muppetlabs.com/~breadbox/software/tiny/teensy.html

*/

void parse_asm_line(char **stream, char *label, char *opcode, char *op1, char *op2, int line_no, int str_size);
bool encode_asm(char *opcode, char *op1, char *op2, char *output, int *out_len);


void generate_binary_code(char *assembly_code, elf_contents **program);

