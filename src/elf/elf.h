#pragma once
#include "elf_contents.h"




void perform_elf_test();
void read_elf_file(char *filename);

bool write_elf_file(elf_contents *prog, char *filename, long *bytes_written);
