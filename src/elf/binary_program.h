#pragma once



// stores the assembler's / linker's output, 
// to be written to (or read from) an ELF file.


typedef struct binary_program binary_program;

struct binary_program {
    struct {
      int is_64_bits: 1;             // as opposed to 32 bits
      int is_object_code: 1;         // needs linking
      int is_dynamic_executable: 1;  // should have relocation info
      int is_static_executable: 1;   // does not need dyn linking
    } flags;
    
    // address and size in memory
    unsigned long code_address;
    unsigned long code_size;
    char *code_contents;

    // entry point in memory
    unsigned long code_entry_point;

    // segment for initialized data (stored in elf)
    unsigned long init_data_address;
    unsigned long init_data_size;
    char *init_data_contents;

    // segment for uninitialized data (only mentioned)
    unsigned long zero_data_address;
    unsigned long zero_data_size;
};

