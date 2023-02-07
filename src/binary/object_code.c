#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../options.h"
#include "object_code.h"
#include "elf_format.h"
#include "read_elf.h"
#include "write_elf.h"



void perform_elf_test() {
    // read_elf_file("mcc");
    // read_elf_file("/bin/sh");
    // read_elf_file("./docs/bin/tiny-obj32");
    // read_elf_file("./docs/bin/tiny-obj64");
    // read_elf_file("./docs/bin/tiny-dyn32");
    // read_elf_file("./docs/bin/tiny-dyn64");
    // read_elf_file("./docs/bin/tiny-stat32");
    // read_elf_file("./docs/bin/tiny-stat64");

    // then we should write a test ELF file, and read it with readelf in terminal.
    
    object_code c;
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
    write_elf_file(&c, "demo-obj");

    c.flags.is_object_code = false;
    c.flags.is_static_executable = true;
    c.flags.is_dynamic_executable = false;
    write_elf_file(&c, "demo-stat");

    c.flags.is_object_code = false;
    c.flags.is_static_executable = false;
    c.flags.is_dynamic_executable = true;
    write_elf_file(&c, "demo-dyn");
}


void printf16hex(void *address, int size) {
    char *p = address;
    while (size > 0) {
        printf("    %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x  %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c\n",
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15],
            (p[0] > 32  && p[0]  < 127) ? p[0] : '.',
            (p[1] > 32  && p[1]  < 127) ? p[1] : '.',
            (p[2] > 32  && p[2]  < 127) ? p[2] : '.',
            (p[3] > 32  && p[3]  < 127) ? p[3] : '.',
            (p[4] > 32  && p[4]  < 127) ? p[4] : '.',
            (p[5] > 32  && p[5]  < 127) ? p[5] : '.',
            (p[6] > 32  && p[6]  < 127) ? p[6] : '.',
            (p[7] > 32  && p[7]  < 127) ? p[7] : '.',
            (p[8] > 32  && p[8]  < 127) ? p[8] : '.',
            (p[9] > 32  && p[9]  < 127) ? p[9] : '.',
            (p[10] > 32 && p[10] < 127) ? p[10] : '.',
            (p[11] > 32 && p[11] < 127) ? p[11] : '.',
            (p[12] > 32 && p[12] < 127) ? p[12] : '.',
            (p[13] > 32 && p[13] < 127) ? p[13] : '.',
            (p[14] > 32 && p[14] < 127) ? p[14] : '.',
            (p[15] > 32 && p[15] < 127) ? p[15] : '.'
        );
        p += 16;
        size -= 16;
    }
}

