# ELF info

It seems that ELF files contain the following blocks, one after the other.

* header (diff 32-64)
* program headers (grouped sections to load, e.g. may be two or more contiguous sections)
* each section one after the other (i.e. the raw contents)
* section headers (apparently collected while writing the sections)

Note that the first section header is all zeros. The object files have no program entries at all. The static files are generally much larger (e.g. 500K instead of 20K)

## Sample file contents

The following listings from the mini compiler code, reading various ELF files, compiled via gcc. It is using dynamic, static and relocatable file types, both for 32 and 64 bits.

### mcc - the mini compiler

```
ELF file information: [mcc]
  Identity   : 7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  Class      : 2 (64 bits)
  Encoding   : 1 (LSB first)
  File type  : 3 (DYN - executable requiring dyn libraries)
  Machine    : 62
  Version    : 1
  Entry point: 0x2360
  Flags      : 0x0
  Header size: 64
  Program Headers - Num 13, Size 56, Offset 64
    P# Type    File pos   File sz     V.Addr     P.Addr    Mem Sz Algn Flg
     0 PHDR          64       728 0x00000040 0x00000040       728    8 R..
     1 INTER        792        28 0x00000318 0x00000318        28    1 R..
     2 LOAD           0      5616 0x00000000 0x00000000      5616 4096 R..
     3 LOAD        8192     39489 0x00002000 0x00002000     39489 4096 R.X
     4 LOAD       49152     14168 0x0000c000 0x0000c000     14168 4096 R..
     5 LOAD       64760      2608 0x00010cf8 0x00010cf8     20624 4096 RW.
     6 DYN        64776       496 0x00010d08 0x00010d08       496    8 RW.
     7 NOTE         824        48 0x00000338 0x00000338        48    8 R..
     8 NOTE         872        68 0x00000368 0x00000368        68    4 R..
     9 ???          824        48 0x00000338 0x00000338        48    8 R..
    10 ???        56124      1428 0x0000db3c 0x0000db3c      1428    4 R..
    11 ???            0         0 0x00000000 0x00000000         0   16 RW.
    12 ???        64760       776 0x00010cf8 0x00010cf8       776    1 R..
  Section Headers - Num 38, Size 64, Offset 176904  (sh str ndx 37)
    S# Name                 Type        Offset      Size  Algn Flg
     0 (none)               ???              0         0      0 ...
     1 .interp              PROGBITS       792        28      1 .A.
     2 .note.gnu.property   NOTE           824        48      8 .A.
     3 .note.gnu.build-id   NOTE           872        36      4 .A.
     4 .note.ABI-tag        NOTE           908        32      4 .A.
     5 .gnu.hash            ???            944        48      8 .A.
     6 .dynsym              DYNSYM         992       792      8 .A.
     7 .dynstr              STRTAB        1784       347      1 .A.
     8 .gnu.version         ???           2132        66      2 .A.
     9 .gnu.version_r       ???           2200        80      8 .A.
    10 .rela.dyn            RELA          2280      2736      8 .A.
    11 .rela.plt            RELA          5016       600      8 .A.
    12 .init                PROGBITS      8192        27      4 .AX
    13 .plt                 PROGBITS      8224       416     16 .AX
    14 .plt.got             PROGBITS      8640        16     16 .AX
    15 .plt.sec             PROGBITS      8656       400     16 .AX
    16 .text                PROGBITS      9056     38612     16 .AX
    17 .fini                PROGBITS     47668        13      4 .AX
    18 .rodata              PROGBITS     49152      6970      8 .A.
    19 .eh_frame_hdr        PROGBITS     56124      1428      4 .A.
    20 .eh_frame            PROGBITS     57552      5768      8 .A.
    21 .init_array          ???          64760         8      8 WA.
    22 .fini_array          ???          64768         8      8 WA.
    23 .dynamic             DYNAMIC      64776       496      8 WA.
    24 .got                 PROGBITS     65272       264      8 WA.
    25 .data                PROGBITS     65536      1832     32 WA.
    26 .bss                 NOBITS       67368     17992     32 WA.
    27 .comment             PROGBITS     67368        43      1 ...
    28 .debug_aranges       PROGBITS     67411      1184      1 ...
    29 .debug_info          PROGBITS     68595     59179      1 ...
    30 .debug_abbrev        PROGBITS    127774     10728      1 ...
    31 .debug_line          PROGBITS    138502     16010      1 ...
    32 .debug_str           PROGBITS    154512      8683      1 ...
    33 .debug_line_str      PROGBITS    163195      1011      1 ...
    34 .debug_rnglists      PROGBITS    164206       102      1 ...
    35 .symtab              SYMTAB      164312      7032      8 ...
    36 .strtab              STRTAB      171344      5176      1 ...
    37 .shstrtab            STRTAB      176520       378      1 ...
```

### Object code

```
ELF file information: [./docs/bin/tiny-obj32]
  Identity   : 7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00
  Class      : 1 (32 bits)
  Encoding   : 1 (LSB first)
  File type  : 1 (REL - object code)
  Machine    : 3
  Version    : 1
  Entry point: 0x0
  Flags      : 0x0
  Header size: 52
  Program Headers - Num 0, Size 0, Offset 0
  Section Headers - Num 14, Size 40, Offset 532  (sh str ndx 13)
    S# Name                 Type        Offset      Size  Algn Flg
     0 (none)               ???              0         0      0 ...
     1 .group               ???             52         8      4 ...
     2 .text                PROGBITS        60        20      1 .AX
     3 .rel.text            REL            376        16      4 ...
     4 .data                PROGBITS        80         0      1 WA.
     5 .bss                 NOBITS          80         0      1 WA.
     6 .text.__x86.get_pc_thunk.ax PROGBITS        80         4      1 .AX
     7 .comment             PROGBITS        84        44      1 ...
     8 .note.GNU-stack      PROGBITS       128         0      1 ...
     9 .eh_frame            PROGBITS       128        76      4 .A.
    10 .rel.eh_frame        REL            392        16      4 ...
    11 .symtab              SYMTAB         204       112      4 ...
    12 .strtab              STRTAB         316        57      1 ...
    13 .shstrtab            STRTAB         408       122      1 ...

ELF file information: [./docs/bin/tiny-obj64]
  Identity   : 7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  Class      : 2 (64 bits)
  Encoding   : 1 (LSB first)
  File type  : 1 (REL - object code)
  Machine    : 62
  Version    : 1
  Entry point: 0x0
  Flags      : 0x0
  Header size: 64
  Program Headers - Num 0, Size 0, Offset 0
  Section Headers - Num 12, Size 64, Offset 456  (sh str ndx 11)
    S# Name                 Type        Offset      Size  Algn Flg
     0 (none)               ???              0         0      0 ...
     1 .text                PROGBITS        64        15      1 .AX
     2 .data                PROGBITS        79         0      1 WA.
     3 .bss                 NOBITS          79         0      1 WA.
     4 .comment             PROGBITS        79        44      1 ...
     5 .note.GNU-stack      PROGBITS       123         0      1 ...
     6 .note.gnu.property   NOTE           128        32      8 .A.
     7 .eh_frame            PROGBITS       160        56      8 .A.
     8 .rela.eh_frame       RELA           328        24      8 ...
     9 .symtab              SYMTAB         216        96      8 ...
    10 .strtab              STRTAB         312        13      1 ...
    11 .shstrtab            STRTAB         352       103      1 ...
```

### Dynamicly linked executables

```
ELF file information: [./docs/bin/tiny-dyn32]
  Identity   : 7f 45 4c 46 01 01 01 00 00 00 00 00 00 00 00 00
  Class      : 1 (32 bits)
  Encoding   : 1 (LSB first)
  File type  : 3 (DYN - executable requiring dyn libraries)
  Machine    : 3
  Version    : 1
  Entry point: 0x1060
  Flags      : 0x0
  Header size: 52
  Program Headers - Num 11, Size 32, Offset 52
    P# Type    File pos   File sz     V.Addr     P.Addr    Mem Sz Algn Flg
     0 PHDR          52       352 0x00000034 0x00000034       352    4 R..
     1 INTER        404        19 0x00000194 0x00000194        19    1 R..
     2 LOAD           0       924 0x00000000 0x00000000       924 4096 R..
     3 LOAD        4096       448 0x00001000 0x00001000       448 4096 R.X
     4 LOAD        8192       216 0x00002000 0x00002000       216 4096 R..
     5 LOAD       11996       300 0x00003edc 0x00003edc       304 4096 RW.
     6 DYN        12004       248 0x00003ee4 0x00003ee4       248    4 RW.
     7 NOTE         424        68 0x000001a8 0x000001a8        68    4 R..
     8 ???         8200        52 0x00002008 0x00002008        52    4 R..
     9 ???            0         0 0x00000000 0x00000000         0   16 RW.
    10 ???        11996       292 0x00003edc 0x00003edc       292    1 R..
  Section Headers - Num 29, Size 40, Offset 13748  (sh str ndx 28)
    S# Name                 Type        Offset      Size  Algn Flg
     0 (none)               ???              0         0      0 ...
     1 .interp              PROGBITS       404        19      1 .A.
     2 .note.gnu.build-id   NOTE           424        36      4 .A.
     3 .note.ABI-tag        NOTE           460        32      4 .A.
     4 .gnu.hash            ???            492        32      4 .A.
     5 .dynsym              DYNSYM         524       112      4 .A.
     6 .dynstr              STRTAB         636       151      1 .A.
     7 .gnu.version         ???            788        14      2 .A.
     8 .gnu.version_r       ???            804        48      4 .A.
     9 .rel.dyn             REL            852        64      4 .A.
    10 .rel.plt             REL            916         8      4 .A.
    11 .init                PROGBITS      4096        36      4 .AX
    12 .plt                 PROGBITS      4144        32     16 .AX
    13 .plt.got             PROGBITS      4176         8      8 .AX
    14 .text                PROGBITS      4192       325     16 .AX
    15 .fini                PROGBITS      4520        24      4 .AX
    16 .rodata              PROGBITS      8192         8      4 .A.
    17 .eh_frame_hdr        PROGBITS      8200        52      4 .A.
    18 .eh_frame            PROGBITS      8252       156      4 .A.
    19 .init_array          ???          11996         4      4 WA.
    20 .fini_array          ???          12000         4      4 WA.
    21 .dynamic             DYNAMIC      12004       248      4 WA.
    22 .got                 PROGBITS     12252        36      4 WA.
    23 .data                PROGBITS     12288         8      4 WA.
    24 .bss                 NOBITS       12296         4      1 WA.
    25 .comment             PROGBITS     12296        43      1 ...
    26 .symtab              SYMTAB       12340       624      4 ...
    27 .strtab              STRTAB       12964       530      1 ...
    28 .shstrtab            STRTAB       13494       252      1 ...

ELF file information: [./docs/bin/tiny-dyn64]
  Identity   : 7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00
  Class      : 2 (64 bits)
  Encoding   : 1 (LSB first)
  File type  : 3 (DYN - executable requiring dyn libraries)
  Machine    : 62
  Version    : 1
  Entry point: 0x1040
  Flags      : 0x0
  Header size: 64
  Program Headers - Num 13, Size 56, Offset 64
    P# Type    File pos   File sz     V.Addr     P.Addr    Mem Sz Algn Flg
     0 PHDR          64       728 0x00000040 0x00000040       728    8 R..
     1 INTER        792        28 0x00000318 0x00000318        28    1 R..
     2 LOAD           0      1520 0x00000000 0x00000000      1520 4096 R..
     3 LOAD        4096       325 0x00001000 0x00001000       325 4096 R.X
     4 LOAD        8192       196 0x00002000 0x00002000       196 4096 R..
     5 LOAD       11760       544 0x00003df0 0x00003df0       552 4096 RW.
     6 DYN        11776       448 0x00003e00 0x00003e00       448    8 RW.
     7 NOTE         824        48 0x00000338 0x00000338        48    8 R..
     8 NOTE         872        68 0x00000368 0x00000368        68    4 R..
     9 ???          824        48 0x00000338 0x00000338        48    8 R..
    10 ???         8196        44 0x00002004 0x00002004        44    4 R..
    11 ???            0         0 0x00000000 0x00000000         0   16 RW.
    12 ???        11760       528 0x00003df0 0x00003df0       528    1 R..
  Section Headers - Num 29, Size 64, Offset 13920  (sh str ndx 28)
    S# Name                 Type        Offset      Size  Algn Flg
     0 (none)               ???              0         0      0 ...
     1 .interp              PROGBITS       792        28      1 .A.
     2 .note.gnu.property   NOTE           824        48      8 .A.
     3 .note.gnu.build-id   NOTE           872        36      4 .A.
     4 .note.ABI-tag        NOTE           908        32      4 .A.
     5 .gnu.hash            ???            944        36      8 .A.
     6 .dynsym              DYNSYM         984       144      8 .A.
     7 .dynstr              STRTAB        1128       136      1 .A.
     8 .gnu.version         ???           1264        12      2 .A.
     9 .gnu.version_r       ???           1280        48      8 .A.
    10 .rela.dyn            RELA          1328       192      8 .A.
    11 .init                PROGBITS      4096        27      4 .AX
    12 .plt                 PROGBITS      4128        16     16 .AX
    13 .plt.got             PROGBITS      4144        16     16 .AX
    14 .text                PROGBITS      4160       248     16 .AX
    15 .fini                PROGBITS      4408        13      4 .AX
    16 .rodata              PROGBITS      8192         4      4 .A.
    17 .eh_frame_hdr        PROGBITS      8196        44      4 .A.
    18 .eh_frame            PROGBITS      8240       148      8 .A.
    19 .init_array          ???          11760         8      8 WA.
    20 .fini_array          ???          11768         8      8 WA.
    21 .dynamic             DYNAMIC      11776       448      8 WA.
    22 .got                 PROGBITS     12224        64      8 WA.
    23 .data                PROGBITS     12288        16      8 WA.
    24 .bss                 NOBITS       12304         8      1 WA.
    25 .comment             PROGBITS     12304        43      1 ...
    26 .symtab              SYMTAB       12352       840      8 ...
    27 .strtab              STRTAB       13192       457      1 ...
    28 .shstrtab            STRTAB       13649       268      1 ...
```

### Statically linked executables

```
ELF file information: [./docs/bin/tiny-stat32]
  Identity   : 7f 45 4c 46 01 01 01 03 00 00 00 00 00 00 00 00
  Class      : 1 (32 bits)
  Encoding   : 1 (LSB first)
  File type  : 2 (EXEC - statically linked executable)
  Machine    : 3
  Version    : 1
  Entry point: 0x80495b0
  Flags      : 0x0
  Header size: 52
  Program Headers - Num 8, Size 32, Offset 52
    P# Type    File pos   File sz     V.Addr     P.Addr    Mem Sz Algn Flg
     0 LOAD           0       488 0x08048000 0x08048000       488 4096 R..
     1 LOAD        4096    453520 0x08049000 0x08049000    453520 4096 R.X
     2 LOAD      458752    203059 0x080b8000 0x080b8000    203059 4096 R..
     3 LOAD      662696     13904 0x080eaca8 0x080eaca8     25900 4096 RW.
     4 NOTE         308        68 0x08048134 0x08048134        68    4 R..
     5 TLS       662696        16 0x080eaca8 0x080eaca8        52    4 R..
     6 ???            0         0 0x00000000 0x00000000         0   16 RW.
     7 ???       662696      9048 0x080eaca8 0x080eaca8      9048    1 R..
  Section Headers - Num 29, Size 40, Offset 745384  (sh str ndx 28)
    S# Name                 Type        Offset      Size  Algn Flg
     0 (none)               ???              0         0      0 ...
     1 .note.gnu.build-id   NOTE           308        36      4 .A.
     2 .note.ABI-tag        NOTE           344        32      4 .A.
     3 .rel.plt             REL            376       112      4 .A.
     4 .init                PROGBITS      4096        36      4 .AX
     5 .plt                 PROGBITS      4136       112      8 .AX
     6 .text                PROGBITS      4256    450343     16 .AX
     7 __libc_freeres_fn    PROGBITS    454608      2981     16 .AX
     8 .fini                PROGBITS    457592        24      4 .AX
     9 .rodata              PROGBITS    458752    114752     32 .A.
    10 .eh_frame            PROGBITS    573504     88044      4 .A.
    11 .gcc_except_table    PROGBITS    661548       263      1 .A.
    12 .tdata               PROGBITS    662696        16      4 WA.
    13 .tbss                NOBITS      662712        36      4 WA.
    14 .init_array          ???         662712         4      4 WA.
    15 .fini_array          ???         662716         4      4 WA.
    16 .data.rel.ro         PROGBITS    662720      8948     32 WA.
    17 .got                 PROGBITS    671668        64      4 WA.
    18 .got.plt             PROGBITS    671744        68      4 WA.
    19 .data                PROGBITS    671840      3752     32 WA.
    20 __libc_subfreeres    PROGBITS    675592        36      4 WA.
    21 __libc_IO_vtables    PROGBITS    675648       948     32 WA.
    22 __libc_atexit        PROGBITS    676596         4      4 WA.
    23 .bss                 NOBITS      676600     11972     32 WA.
    24 __libc_freeres_ptrs  NOBITS      676600        16      4 WA.
    25 .comment             PROGBITS    676600        43      1 ...
    26 .symtab              SYMTAB      676644     39200      4 ...
    27 .strtab              STRTAB      715844     29245      1 ...
    28 .shstrtab            STRTAB      745089       295      1 ...

ELF file information: [./docs/bin/tiny-stat64]
  Identity   : 7f 45 4c 46 02 01 01 03 00 00 00 00 00 00 00 00
  Class      : 2 (64 bits)
  Encoding   : 1 (LSB first)
  File type  : 2 (EXEC - statically linked executable)
  Machine    : 62
  Version    : 1
  Entry point: 0x401620
  Flags      : 0x0
  Header size: 64
  Program Headers - Num 10, Size 56, Offset 64
    P# Type    File pos   File sz     V.Addr     P.Addr    Mem Sz Algn Flg
     0 LOAD           0      1320 0x00400000 0x00400000      1320 4096 R..
     1 LOAD        4096    615613 0x00401000 0x00401000    615613 4096 R.X
     2 LOAD      622592    164982 0x00498000 0x00498000    164982 4096 R..
     3 LOAD      788400     23264 0x004c17b0 0x004c17b0     46224 4096 RW.
     4 NOTE         624        48 0x00400270 0x00400270        48    8 R..
     5 NOTE         672        68 0x004002a0 0x004002a0        68    4 R..
     6 TLS       788400        32 0x004c17b0 0x004c17b0       104    8 R..
     7 ???          624        48 0x00400270 0x00400270        48    8 R..
     8 ???            0         0 0x00000000 0x00000000         0   16 RW.
     9 ???       788400     14416 0x004c17b0 0x004c17b0     14416    1 R..
  Section Headers - Num 32, Size 64, Offset 898128  (sh str ndx 31)
    S# Name                 Type        Offset      Size  Algn Flg
     0 (none)               ???              0         0      0 ...
     1 .note.gnu.property   NOTE           624        48      8 .A.
     2 .note.gnu.build-id   NOTE           672        36      4 .A.
     3 .note.ABI-tag        NOTE           708        32      4 .A.
     4 .rela.plt            RELA           744       576      8 .A.
     5 .init                PROGBITS      4096        27      4 .AX
     6 .plt                 PROGBITS      4128       384     16 .AX
     7 .text                PROGBITS      4544    609816     64 .AX
     8 __libc_freeres_fn    PROGBITS    614368      5328     16 .AX
     9 .fini                PROGBITS    619696        13      4 .AX
    10 .rodata              PROGBITS    622592    117468     32 .A.
    11 .stapsdt.base        PROGBITS    740060         1      1 .A.
    12 .eh_frame            PROGBITS    740064     47248      8 .A.
    13 .gcc_except_table    PROGBITS    787312       262      1 .A.
    14 .tdata               PROGBITS    788400        32      8 WA.
    15 .tbss                NOBITS      788432        72      8 WA.
    16 .init_array          ???         788432         8      8 WA.
    17 .fini_array          ???         788440         8      8 WA.
    18 .data.rel.ro         PROGBITS    788448     14216     32 WA.
    19 .got                 PROGBITS    802664       152      8 WA.
    20 .got.plt             PROGBITS    802816       216      8 WA.
    21 .data                PROGBITS    803040      6632     32 WA.
    22 __libc_subfreeres    PROGBITS    809672        72      8 WA.
    23 __libc_IO_vtables    PROGBITS    809760      1896     32 WA.
    24 __libc_atexit        PROGBITS    811656         8      8 WA.
    25 .bss                 NOBITS      811664     22912     32 WA.
    26 __libc_freeres_ptrs  NOBITS      811664        32      8 WA.
    27 .comment             PROGBITS    811664        43      1 ...
    28 .note.stapsdt        NOTE        811708      5704      4 ...
    29 .symtab              SYMTAB      817416     50112      8 ...
    30 .strtab              STRTAB      867528     30257      1 ...
    31 .shstrtab            STRTAB      897785       343      1 ...
```