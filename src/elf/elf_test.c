#include <string.h>
#include <stdbool.h>
#include "binary_program.h"
#include "elf.h"


void perform_elf_test() {
    read_elf_file("mcc");
    read_elf_file("/bin/sh");
    read_elf_file("./docs/bin/tiny-obj32");
    read_elf_file("./docs/bin/tiny-obj64");
    read_elf_file("./docs/bin/tiny-dyn32");
    read_elf_file("./docs/bin/tiny-dyn64");
    read_elf_file("./docs/bin/tiny-stat32");
    read_elf_file("./docs/bin/tiny-stat64");

    binary_program c;
    memset(&c, 0, sizeof(c));
    c.code_address = 0x10;
    c.code_size = 0x10;
    c.code_contents = "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC";
    c.init_data_address = 0x30;
    c.init_data_size = 0x10;
    c.init_data_contents = "DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD";
    c.zero_data_address = 0x50;
    c.zero_data_size = 0x10;
    c.code_entry_point = 0x13;
    c.flags.is_64_bits = true;

    c.flags.is_object_code = true;
    c.flags.is_static_executable = false;
    c.flags.is_dynamic_executable = false;
    write_elf_file(&c, "demo-obj", NULL);

    c.flags.is_object_code = false;
    c.flags.is_static_executable = true;
    c.flags.is_dynamic_executable = false;
    write_elf_file(&c, "demo-stat", NULL);

    c.flags.is_object_code = false;
    c.flags.is_static_executable = false;
    c.flags.is_dynamic_executable = true;
    write_elf_file(&c, "demo-dyn", NULL);
}
