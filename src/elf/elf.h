#pragma once
#include "binary_program.h"




void perform_elf_test();
void read_elf_file(char *filename);

bool write_elf_file(binary_program *prog, char *filename, long *bytes_written);
