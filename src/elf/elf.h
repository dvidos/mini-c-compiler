#pragma once
#include "elf_contents.h"


void perform_elf_test();
elf_contents *read_elf_file(char *filename, elf_contents *contents);
bool write_elf_file(elf_contents *prog, char *filename);
