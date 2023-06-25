#pragma once
#include <stdint.h>

// see https://www.cs.cmu.edu/afs/cs/academic/class/15213-f00/docs/elf.pdf
// and https://courses.cs.washington.edu/courses/cse351/12wi/supp-docs/abi.pdf



// base elf-32 sizes
typedef uint32_t elf32_address;
typedef uint16_t elf32_half_word;
typedef uint32_t elf32_offset;
typedef  int32_t elf32_signed_word;
typedef uint32_t elf32_word;

// base elf-64 sizes
typedef uint64_t elf64_address;
typedef uint16_t elf64_half_word;
typedef  int16_t elf64_signed_half;
typedef uint64_t elf64_offset;
typedef  int32_t elf64_signed_word;
typedef uint32_t elf64_word;
typedef uint64_t elf64_extra_word;
typedef  int64_t elf64_signed_extra_word;

//-----------------------------------------------------------------

typedef struct elf32_header {
  unsigned char identity[16];
  elf32_half_word file_type; // see ET_ constants
  elf32_half_word machine;
  elf32_word      version;   // 1 == current
  elf32_address   entry_point;  // entry point for executables
  elf32_offset    prog_headers_offset;
  elf32_offset    section_headers_offset;
  elf32_word      flags;
  elf32_half_word elf_header_size;
  elf32_half_word prog_headers_entry_size;
  elf32_half_word prog_headers_entries;
  elf32_half_word section_headers_entry_size;
  elf32_half_word section_headers_entries;
  elf32_half_word section_headers_strings_entry;
} elf32_header;

typedef struct elf64_header {
  unsigned char   identity[16];
  elf64_half_word file_type;  // see the ET_ constants
  elf64_half_word machine;
  elf64_word      version;    // 1 = current
  elf64_address   entry_point;  // entry point for executables
  elf64_offset    prog_headers_offset;
  elf64_offset    section_headers_offset;
  elf64_word      flags;
  elf64_half_word elf_header_size;
  elf64_half_word prog_headers_entry_size;
  elf64_half_word prog_headers_entries;
  elf64_half_word section_headers_entry_size;
  elf64_half_word section_headers_entries;
  elf64_half_word section_headers_strings_entry;
} elf64_header;

// for identity bytes
#define ELF_IDENTITY_MAGIC4   0
#define ELF_IDENTITY_CLASS    4
#define ELF_IDENTITY_DATA     5
#define ELF_IDENTITY_VERSION  6
#define ELF_IDENTITY_OS_ABI   7
#define ELF_IDENTITY_PAD      8

// elf file type
#define ELF_TYPE_NONE   0
#define ELF_TYPE_REL    1
#define ELF_TYPE_EXEC   2
#define ELF_TYPE_DYN    3
#define ELF_TYPE_CORE   4

// identity[4], class
#define ELF_CLASS_32  1
#define ELF_CLASS_64  2

// identity[5], data endianness
#define ELF_DATA2_LSB 1  // intel, our case
#define ELF_DATA2_MSB 2

// identity[6], version
#define ELF_VERSION_CURRENT  1

// identity[7], OS ABI
#define ELF_OSABI_SYSV   0
#define ELF_OSABI_LINUX  3  // this was interpreted by "readelf",c creating expectations...

// elf header machine type
#define ELF_MACHINE_386         3
#define ELF_MACHINE_X86_64     62


//-----------------------------------------------------------------

typedef struct elf32_prog_header {
  elf32_word    type;
  elf32_offset  file_offset;
  elf32_address virt_address;
  elf32_address phys_address;
  elf32_word    file_size;
  elf32_word    memory_size;
  elf32_word    flags;
  elf32_word    align;
} elf32_prog_header;

typedef struct elf64_prog_header {
  elf64_word       type;
  elf64_word       flags;
  elf64_offset     file_offset;
  elf64_address    virt_address;
  elf64_address    phys_address;
  elf64_extra_word file_size;
  elf64_extra_word memory_size;
  elf64_extra_word align;
} elf64_prog_header;

// program header type
#define PROG_TYPE_NULL    0 
#define PROG_TYPE_LOAD    1  // to be loaded into memory
#define PROG_TYPE_DYNAMIC 2  // dyn linkage info
#define PROG_TYPE_INTERP  3
#define PROG_TYPE_NOTE    4
#define PROG_TYPE_SHLIB   5
#define PROG_TYPE_PHDR    6
#define PROG_TYPE_TLS     7

// program header flags
#define PROG_FLAGS_READ     0x4
#define PROG_FLAGS_WRITE    0x2
#define PROG_FLAGS_EXECUTE  0x1

//-----------------------------------------------------------------

typedef struct elf32_section_header {
  elf32_word    name;          // section name, index in string table
  elf32_word    type;
  elf32_word    flags;
  elf32_address virt_address;  // virtual address in memory
  elf32_offset  file_offset;
  elf32_word    size;          // size of section, in file or memory
  elf32_word    link;
  elf32_word    info;
  elf32_word    address_alignment; // e.g. 4 or 1
  elf32_word    entry_size;
} elf32_section_header;

typedef struct elf64_section_header {
  elf64_word        name;
  elf64_word        type;
  elf64_extra_word  flags;
  elf64_address     virt_address;
  elf64_offset      file_offset;
  elf64_extra_word  size;
  elf64_word        link; // for rela, this is symtab, for symtab, this is strtab, 
  elf64_word        info; // for rela, this is pointed section, for symtab, first global symbol index
  elf64_extra_word  address_alignment;
  elf64_extra_word  entry_size;
} elf64_section_header;

// section types
#define SECTION_TYPE_NULL     0
#define SECTION_TYPE_PROGBITS 1   // various
#define SECTION_TYPE_SYMTAB   2   // symbols table for debugging
#define SECTION_TYPE_STRTAB   3   // strings table
#define SECTION_TYPE_RELA     4
#define SECTION_TYPE_HASH     5   // symbols table hash
#define SECTION_TYPE_DYNAMIC  6
#define SECTION_TYPE_NOTE     7
#define SECTION_TYPE_NOBITS   8   // no file size, but memory size (e.g. .bss)
#define SECTION_TYPE_REL      9
#define SECTION_TYPE_SHLIB    10
#define SECTION_TYPE_DYNSYM   11
#define SECTION_TYPE_NUM      12

// section flags
#define SECTION_FLAGS_WRITE          0x1    // should be writable (e.g. data)
#define SECTION_FLAGS_ALLOC          0x2    // should occupy memory during execution
#define SECTION_FLAGS_EXECINSTR      0x4    // contains executable instructions

// special section indexes
#define SHN_UNDEF          0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC    0xff00
#define SHN_HIPROC    0xff1f
#define SHN_LIVEPATCH 0xff20
#define SHN_ABS       0xfff1
#define SHN_COMMON    0xfff2
#define SHN_HIRESERVE 0xffff

//-----------------------------------------------------------------

 // This info is needed when parsing the symbol table
#define STB_LOCAL  0
#define STB_GLOBAL 1
#define STB_WEAK   2

#define STT_NOTYPE  0
#define STT_OBJECT  1
#define STT_FUNC    2
#define STT_SECTION 3
#define STT_FILE    4
#define STT_COMMON  5
#define STT_TLS     6

#define ELF32_ST_BIND(x)           ((x) >> 4)
#define ELF32_ST_TYPE(x)           ((x) & 0xf)
#define ELF32_ST_INFO(bind, type)  (((bind) << 4) + ((type) & 0xf) )
#define ELF64_ST_BIND(x)           ((x) >> 4)
#define ELF64_ST_TYPE(x)           ((x) & 0xf)
#define ELF64_ST_INFO(bind, type)  (((bind) << 4) + ((type) & 0xf) )


typedef struct elf32_sym {
  elf32_word      st_name;   // Symbol name, index in string tbl
  elf32_address   st_value;  
  elf32_word      st_size;
  unsigned char   st_info;   // Type and binding attributes
  unsigned char   st_other;
  elf32_half_word st_shndx;
} elf32_sym;

typedef struct elf64_sym {
  elf64_word       st_name;   // Symbol name, index in string tbl
  unsigned char    st_info;   // Type and binding attributes
  unsigned char    st_other;  // No defined meaning, 0
  elf64_half_word  st_shndx;  // Associated section index
  elf64_address    st_value;  // Value of the symbol
  elf64_extra_word st_size;   // Associated symbol size
} elf64_sym;

//-----------------------------------------------------------------

// The following are used with relocations
#define ELF32_R_SYM(x)    ((x) >> 8)
#define ELF32_R_TYPE(x)   ((x) & 0xff)
#define ELF32_R_INFO(sym,type) (((sym) << 8) + (unsigned char)(type))

#define ELF64_R_SYM(i)   ((i) >> 32)
#define ELF64_R_TYPE(i)   ((i) & 0xffffffff)
#define ELF64_R_INFO(sym, type) ((((elf64_extra_word)(sym)) << 32) + (type & 0xffffffff))

typedef struct elf32_rela {
  elf32_address r_offset;
  elf32_word r_info;
  elf32_signed_word r_addend;
} elf32_rela;

typedef struct elf64_rela {
  elf64_address r_offset;  // Location at which to apply the action
  elf64_extra_word r_info;  // index and type of relocation
  elf64_signed_extra_word r_addend;  // Constant addend used to compute value
} elf64_rela;



//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

/*
 // This is the info that is needed to parse the dynamic section of the file
#define DT_NULL  0
#define DT_NEEDED 1
#define DT_PLTRELSZ 2
#define DT_PLTGOT 3
#define DT_HASH  4
#define DT_STRTAB 5
#define DT_SYMTAB 6
#define DT_RELA  7
#define DT_RELASZ 8
#define DT_RELAENT 9
#define DT_STRSZ 10
#define DT_SYMENT 11
#define DT_INIT  12
#define DT_FINI  13
#define DT_SONAME 14
#define DT_RPATH 15
#define DT_SYMBOLIC 16
#define DT_REL         17
#define DT_RELSZ 18
#define DT_RELENT 19
#define DT_PLTREL 20
#define DT_DEBUG 21
#define DT_TEXTREL 22
#define DT_JMPREL 23
#define DT_ENCODING 32
#define OLD_DT_LOOS 0x60000000
#define DT_LOOS  0x6000000d
#define DT_HIOS  0x6ffff000
#define DT_VALRNGLO 0x6ffffd00
#define DT_VALRNGHI 0x6ffffdff
#define DT_ADDRRNGLO 0x6ffffe00
#define DT_ADDRRNGHI 0x6ffffeff
#define DT_VERSYM 0x6ffffff0
#define DT_RELACOUNT 0x6ffffff9
#define DT_RELCOUNT 0x6ffffffa
#define DT_FLAGS_1 0x6ffffffb
#define DT_VERDEF 0x6ffffffc
#define DT_VERDEFNUM 0x6ffffffd
#define DT_VERNEED 0x6ffffffe
#define DT_VERNEEDNUM 0x6fffffff
#define OLD_DT_HIOS     0x6fffffff
#define DT_LOPROC 0x70000000
#define DT_HIPROC 0x7fffffff

typedef struct dynamic {
  elf32_signed_word d_tag;
  union {
    elf32_signed_word d_val;
    elf32_address d_ptr;
  } d_un;
} elf32_dyn;

typedef struct {
  elf64_signed_extra_word d_tag;   // entry tag value
  union {
    elf64_extra_word d_val;
    elf64_address d_ptr;
  } d_un;
} elf64_dyn;

// The following are used with relocations
#define ELF32_R_SYM(x) ((x) >> 8)
#define ELF32_R_TYPE(x) ((x) & 0xff)

#define ELF64_R_SYM(i)   ((i) >> 32)
#define ELF64_R_TYPE(i)   ((i) & 0xffffffff)

typedef struct elf32_rel {
  elf32_address r_offset;
  elf32_word r_info;
} elf32_rel;

typedef struct elf64_rel {
  elf64_address r_offset;  // Location at which to apply the action
  elf64_extra_word r_info;  // index and type of relocation
} elf64_rel;

typedef struct elf32_rela {
  elf32_address r_offset;
  elf32_word r_info;
  elf32_signed_word r_addend;
} elf32_rela;

typedef struct elf64_rela {
  elf64_address r_offset;  // Location at which to apply the action
  elf64_extra_word r_info;  // index and type of relocation
  elf64_signed_extra_word r_addend;  // Constant addend used to compute value
} elf64_rela;


#define ELF_IDENTITY_BYTES 16

typedef struct elf32_header {
  unsigned char identity[ELF_IDENTITY_BYTES];
  elf32_half_word e_type;
  elf32_half_word e_machine;
  elf32_word e_version;
  elf32_address entry_point;   // Entry point
  elf32_offset prog_headers_offset;
  elf32_offset section_headers_offset;
  elf32_word e_flags;
  elf32_half_word e_ehsize;
  elf32_half_word prog_headers_entry_size;
  elf32_half_word prog_headers_entries;
  elf32_half_word section_headers_entry_size;
  elf32_half_word section_headers_entries;
  elf32_half_word section_headers_strings_entry;
} elf32_header;

typedef struct elf64_header {
  unsigned char identity[ELF_IDENTITY_BYTES];  // ELF "magic number"
  elf64_half_word e_type;
  elf64_half_word e_machine;
  elf64_word e_version;
  elf64_address entry_point;   // Entry point virtual address
  elf64_offset prog_headers_offset;   // Program header table file offset
  elf64_offset section_headers_offset;   // Section header table file offset
  elf64_word e_flags;
  elf64_half_word e_ehsize;
  elf64_half_word prog_headers_entry_size;
  elf64_half_word prog_headers_entries;
  elf64_half_word section_headers_entry_size;
  elf64_half_word section_headers_entries;
  elf64_half_word section_headers_strings_entry;
} elf64_ehdr;




 // sh_type
#define SHT_NULL 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4
#define SHT_HASH 5
#define SHT_DYNAMIC 6
#define SHT_NOTE 7
#define SHT_NOBITS 8
#define SHT_REL  9
#define SHT_SHLIB 10
#define SHT_DYNSYM 11
#define SHT_NUM  12
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff

 // sh_flags
#define SHF_WRITE  0x1
#define SHF_ALLOC  0x2
#define SHF_EXECINSTR  0x4
#define SHF_RELA_LIVEPATCH 0x00100000
#define SHF_RO_AFTER_INIT 0x00200000
#define SHF_MASKPROC  0xf0000000

 // special section indexes
#define SHN_UNDEF 0
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_HIPROC 0xff1f
#define SHN_LIVEPATCH 0xff20
#define SHN_ABS  0xfff1
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff
 
#define EI_MAG0  0   // e_ident[] indexes
#define EI_MAG1  1
#define EI_MAG2  2
#define EI_MAG3  3
#define EI_CLASS 4
#define EI_DATA  5
#define EI_VERSION 6
#define EI_OSABI 7
#define EI_PAD  8

#define ELFMAG0  0x7f   // EI_MAG
#define ELFMAG1  'E'
#define ELFMAG2  'L'
#define ELFMAG3  'F'
#define ELFMAG  "\177ELF"
#define SELFMAG  4

#define ELFCLASSNONE 0   // EI_CLASS
#define ELFCLASS32 1
#define ELFCLASS64 2
#define ELFCLASSNUM 3

#define ELFDATANONE 0   // e_ident[EI_DATA]
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

#define EV_NONE  0   // e_version, EI_VERSION
#define EV_CURRENT 1
#define EV_NUM  2

#define ELFOSABI_NONE 0
#define ELFOSABI_LINUX 3

#ifndef ELF_OSABI
#define ELF_OSABI ELFOSABI_NONE
#endif


#define NT_PRSTATUS 1
#define NT_PRFPREG 2
#define NT_PRPSINFO 3
#define NT_TASKSTRUCT 4
#define NT_AUXV  6

#define NT_SIGINFO      0x53494749
#define NT_FILE         0x46494c45
#define NT_PRXFPREG     0x46e62b7f       // copied from gdb5.1/include/elf/common.h
#define NT_PPC_VMX 0x100   // PowerPC Altivec/VMX registers
#define NT_PPC_SPE 0x101   // PowerPC SPE/EVR registers
#define NT_PPC_VSX 0x102   // PowerPC VSX registers
#define NT_PPC_TAR 0x103   // Target Address Register
#define NT_PPC_PPR 0x104   // Program Priority Register
#define NT_PPC_DSCR 0x105   // Data Stream Control Register
#define NT_PPC_EBB 0x106   // Event Based Branch Registers
#define NT_PPC_PMU 0x107   // Performance Monitor Registers
#define NT_PPC_TM_CGPR 0x108   // TM checkpointed GPR Registers
#define NT_PPC_TM_CFPR 0x109   // TM checkpointed FPR Registers
#define NT_PPC_TM_CVMX 0x10a   // TM checkpointed VMX Registers
#define NT_PPC_TM_CVSX 0x10b   // TM checkpointed VSX Registers
#define NT_PPC_TM_SPR 0x10c   // TM Special Purpose Registers
#define NT_PPC_TM_CTAR 0x10d   // TM checkpointed Target Address Register
#define NT_PPC_TM_CPPR 0x10e   // TM checkpointed Program Priority Register
#define NT_PPC_TM_CDSCR 0x10f   // TM checkpointed Data Stream Control Register
#define NT_PPC_PKEY 0x110   // Memory Protection Keys registers
#define NT_386_TLS 0x200   // i386 TLS slots (struct user_desc)
#define NT_386_IOPERM 0x201   // x86 io permission bitmap (1=deny)
#define NT_X86_XSTATE 0x202   // x86 extended state using xsave
#define NT_S390_HIGH_GPRS 0x300  // s390 upper register halves
#define NT_S390_TIMER 0x301   // s390 timer register
#define NT_S390_TODCMP 0x302   // s390 TOD clock comparator register
#define NT_S390_TODPREG 0x303   // s390 TOD programmable register
#define NT_S390_CTRS 0x304   // s390 control registers
#define NT_S390_PREFIX 0x305   // s390 prefix register
#define NT_S390_LAST_BREAK 0x306  // s390 breaking event address
#define NT_S390_SYSTEM_CALL 0x307  // s390 system call restart data
#define NT_S390_TDB 0x308   // s390 transaction diagnostic block
#define NT_S390_VXRS_LOW 0x309  // s390 vector registers 0-15 upper half
#define NT_S390_VXRS_HIGH 0x30a  // s390 vector registers 16-31
#define NT_S390_GS_CB 0x30b   // s390 guarded storage registers
#define NT_S390_GS_BC 0x30c   // s390 guarded storage broadcast control block
#define NT_S390_RI_CB 0x30d   // s390 runtime instrumentation
#define NT_S390_PV_CPU_DATA 0x30e  // s390 protvirt cpu dump data
#define NT_ARM_VFP 0x400   // ARM VFP/NEON registers
#define NT_ARM_TLS 0x401   // ARM TLS register
#define NT_ARM_HW_BREAK 0x402   // ARM hardware breakpoint registers
#define NT_ARM_HW_WATCH 0x403   // ARM hardware watchpoint registers
#define NT_ARM_SYSTEM_CALL 0x404  // ARM system call number
#define NT_ARM_SVE 0x405   // ARM Scalable Vector Extension registers
#define NT_ARM_PAC_MASK  0x406  // ARM pointer authentication code masks
#define NT_ARM_PACA_KEYS 0x407  // ARM pointer authentication address keys
#define NT_ARM_PACG_KEYS 0x408  // ARM pointer authentication generic key
#define NT_ARM_TAGGED_ADDR_CTRL 0x409  // arm64 tagged address control (prctl())
#define NT_ARM_PAC_ENABLED_KEYS 0x40a  // arm64 ptr auth enabled keys (prctl())
#define NT_ARM_SSVE 0x40b   // ARM Streaming SVE registers
#define NT_ARM_ZA 0x40c   // ARM SME ZA registers
#define NT_ARC_V2 0x600   // ARCv2 accumulator/extra registers
#define NT_VMCOREDD 0x700   // Vmcore Device Dump Note
#define NT_MIPS_DSP 0x800   // MIPS DSP ASE registers
#define NT_MIPS_FP_MODE 0x801   // MIPS floating-point mode
#define NT_MIPS_MSA 0x802   // MIPS SIMD registers
#define NT_LOONGARCH_CPUCFG 0xa00  // LoongArch CPU config registers
#define NT_LOONGARCH_CSR 0xa01  // LoongArch control and status registers
#define NT_LOONGARCH_LSX 0xa02  // LoongArch Loongson SIMD Extension registers
#define NT_LOONGARCH_LASX 0xa03  // LoongArch Loongson Advanced SIMD Extension registers
#define NT_LOONGARCH_LBT 0xa04  // LoongArch Loongson Binary Translation registers

 // Note types with note name "GNU"
#define NT_GNU_PROPERTY_TYPE_0 5

 // Note header in a PT_NOTE section
typedef struct elf32_note {
  elf32_word n_namesz;  // Name size
  elf32_word n_descsz;  // Content size
  elf32_word n_type;   // Content type
} elf32_Nhdr;

 // Note header in a PT_NOTE section
typedef struct elf64_note {
  elf64_word n_namesz;  // Name size
  elf64_word n_descsz;  // Content size
  elf64_word n_type;  // Content type
} elf64_nhdr;


*/

