# ELF dumping

Using dynamic, static and relocatable file types, both for 32 and 64 bits.

```
mini-c-compiler, v0.01
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
     1 PHDR          64       728 0x00000040 0x00000040       728    8 R..
     2 INTER        792        28 0x00000318 0x00000318        28    1 R..
     3 LOAD           0      5616 0x00000000 0x00000000      5616 4096 R..
     4 LOAD        8192     39505 0x00002000 0x00002000     39505 4096 R.X
     5 LOAD       49152     14168 0x0000c000 0x0000c000     14168 4096 R..
     6 LOAD       64760      2608 0x00010cf8 0x00010cf8     20624 4096 RW.
     7 DYN        64776       496 0x00010d08 0x00010d08       496    8 RW.
     8 NOTE         824        48 0x00000338 0x00000338        48    8 R..
     9 NOTE         872        68 0x00000368 0x00000368        68    4 R..
    10 ???          824        48 0x00000338 0x00000338        48    8 R..
    11 ???        56124      1428 0x0000db3c 0x0000db3c      1428    4 R..
    12 ???            0         0 0x00000000 0x00000000         0   16 RW.
    13 ???        64760       776 0x00010cf8 0x00010cf8       776    1 R..
  Section Headers - Num 38, Size 64, Offset 176888  (sh str ndx 37)
    S# Name                 Type        Offset      Size  Algn Flg
     1 (none)               ???              0         0      0 ...
     2 .interp              PROGBITS       792        28      1 .A.
     3 .note.gnu.property   NOTE           824        48      8 .A.
     4 .note.gnu.build-id   NOTE           872        36      4 .A.
     5 .note.ABI-tag        NOTE           908        32      4 .A.
     6 .gnu.hash            ???            944        48      8 .A.
     7 .dynsym              DYNSYM         992       792      8 .A.
     8 .dynstr              STRTAB        1784       347      1 .A.
     9 .gnu.version         ???           2132        66      2 .A.
    10 .gnu.version_r       ???           2200        80      8 .A.
    11 .rela.dyn            RELA          2280      2736      8 .A.
    12 .rela.plt            RELA          5016       600      8 .A.
    13 .init                PROGBITS      8192        27      4 .AX
    14 .plt                 PROGBITS      8224       416     16 .AX
    15 .plt.got             PROGBITS      8640        16     16 .AX
    16 .plt.sec             PROGBITS      8656       400     16 .AX
    17 .text                PROGBITS      9056     38628     16 .AX
    18 .fini                PROGBITS     47684        13      4 .AX
    19 .rodata              PROGBITS     49152      6970      8 .A.
    20 .eh_frame_hdr        PROGBITS     56124      1428      4 .A.
    21 .eh_frame            PROGBITS     57552      5768      8 .A.
    22 .init_array          ???          64760         8      8 WA.
    23 .fini_array          ???          64768         8      8 WA.
    24 .dynamic             DYNAMIC      64776       496      8 WA.
    25 .got                 PROGBITS     65272       264      8 WA.
    26 .data                PROGBITS     65536      1832     32 WA.
    27 .bss                 NOBITS       67368     17992     32 WA.
    28 .comment             PROGBITS     67368        43      1 ...
    29 .debug_aranges       PROGBITS     67411      1184      1 ...
    30 .debug_info          PROGBITS     68595     59179      1 ...
    31 .debug_abbrev        PROGBITS    127774     10728      1 ...
    32 .debug_line          PROGBITS    138502     15994      1 ...
    33 .debug_str           PROGBITS    154496      8683      1 ...
    34 .debug_line_str      PROGBITS    163179      1011      1 ...
    35 .debug_rnglists      PROGBITS    164190       102      1 ...
    36 .symtab              SYMTAB      164296      7032      8 ...
    37 .strtab              STRTAB      171328      5176      1 ...
    38 .shstrtab            STRTAB      176504       378      1 ...

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
     1 (none)               ???              0         0      0 ...
     2 .group               ???             52         8      4 ...
     3 .text                PROGBITS        60        20      1 .AX
     4 .rel.text            REL            376        16      4 ...
     5 .data                PROGBITS        80         0      1 WA.
     6 .bss                 NOBITS          80         0      1 WA.
     7 .text.__x86.get_pc_thunk.ax PROGBITS        80         4      1 .AX
     8 .comment             PROGBITS        84        44      1 ...
     9 .note.GNU-stack      PROGBITS       128         0      1 ...
    10 .eh_frame            PROGBITS       128        76      4 .A.
    11 .rel.eh_frame        REL            392        16      4 ...
    12 .symtab              SYMTAB         204       112      4 ...
    13 .strtab              STRTAB         316        57      1 ...
    14 .shstrtab            STRTAB         408       122      1 ...

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
     1 (none)               ???              0         0      0 ...
     2 .text                PROGBITS        64        15      1 .AX
     3 .data                PROGBITS        79         0      1 WA.
     4 .bss                 NOBITS          79         0      1 WA.
     5 .comment             PROGBITS        79        44      1 ...
     6 .note.GNU-stack      PROGBITS       123         0      1 ...
     7 .note.gnu.property   NOTE           128        32      8 .A.
     8 .eh_frame            PROGBITS       160        56      8 .A.
     9 .rela.eh_frame       RELA           328        24      8 ...
    10 .symtab              SYMTAB         216        96      8 ...
    11 .strtab              STRTAB         312        13      1 ...
    12 .shstrtab            STRTAB         352       103      1 ...

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
     1 PHDR          52       352 0x00000034 0x00000034       352    4 R..
     2 INTER        404        19 0x00000194 0x00000194        19    1 R..
     3 LOAD           0       924 0x00000000 0x00000000       924 4096 R..
     4 LOAD        4096       448 0x00001000 0x00001000       448 4096 R.X
     5 LOAD        8192       216 0x00002000 0x00002000       216 4096 R..
     6 LOAD       11996       300 0x00003edc 0x00003edc       304 4096 RW.
     7 DYN        12004       248 0x00003ee4 0x00003ee4       248    4 RW.
     8 NOTE         424        68 0x000001a8 0x000001a8        68    4 R..
     9 ???         8200        52 0x00002008 0x00002008        52    4 R..
    10 ???            0         0 0x00000000 0x00000000         0   16 RW.
    11 ???        11996       292 0x00003edc 0x00003edc       292    1 R..
  Section Headers - Num 29, Size 40, Offset 13748  (sh str ndx 28)
    S# Name                 Type        Offset      Size  Algn Flg
     1 (none)               ???              0         0      0 ...
     2 .interp              PROGBITS       404        19      1 .A.
     3 .note.gnu.build-id   NOTE           424        36      4 .A.
     4 .note.ABI-tag        NOTE           460        32      4 .A.
     5 .gnu.hash            ???            492        32      4 .A.
     6 .dynsym              DYNSYM         524       112      4 .A.
     7 .dynstr              STRTAB         636       151      1 .A.
     8 .gnu.version         ???            788        14      2 .A.
     9 .gnu.version_r       ???            804        48      4 .A.
    10 .rel.dyn             REL            852        64      4 .A.
    11 .rel.plt             REL            916         8      4 .A.
    12 .init                PROGBITS      4096        36      4 .AX
    13 .plt                 PROGBITS      4144        32     16 .AX
    14 .plt.got             PROGBITS      4176         8      8 .AX
    15 .text                PROGBITS      4192       325     16 .AX
    16 .fini                PROGBITS      4520        24      4 .AX
    17 .rodata              PROGBITS      8192         8      4 .A.
    18 .eh_frame_hdr        PROGBITS      8200        52      4 .A.
    19 .eh_frame            PROGBITS      8252       156      4 .A.
    20 .init_array          ???          11996         4      4 WA.
    21 .fini_array          ???          12000         4      4 WA.
    22 .dynamic             DYNAMIC      12004       248      4 WA.
    23 .got                 PROGBITS     12252        36      4 WA.
    24 .data                PROGBITS     12288         8      4 WA.
    25 .bss                 NOBITS       12296         4      1 WA.
    26 .comment             PROGBITS     12296        43      1 ...
    27 .symtab              SYMTAB       12340       624      4 ...
    28 .strtab              STRTAB       12964       530      1 ...
    29 .shstrtab            STRTAB       13494       252      1 ...

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
     1 PHDR          64       728 0x00000040 0x00000040       728    8 R..
     2 INTER        792        28 0x00000318 0x00000318        28    1 R..
     3 LOAD           0      1520 0x00000000 0x00000000      1520 4096 R..
     4 LOAD        4096       325 0x00001000 0x00001000       325 4096 R.X
     5 LOAD        8192       196 0x00002000 0x00002000       196 4096 R..
     6 LOAD       11760       544 0x00003df0 0x00003df0       552 4096 RW.
     7 DYN        11776       448 0x00003e00 0x00003e00       448    8 RW.
     8 NOTE         824        48 0x00000338 0x00000338        48    8 R..
     9 NOTE         872        68 0x00000368 0x00000368        68    4 R..
    10 ???          824        48 0x00000338 0x00000338        48    8 R..
    11 ???         8196        44 0x00002004 0x00002004        44    4 R..
    12 ???            0         0 0x00000000 0x00000000         0   16 RW.
    13 ???        11760       528 0x00003df0 0x00003df0       528    1 R..
  Section Headers - Num 29, Size 64, Offset 13920  (sh str ndx 28)
    S# Name                 Type        Offset      Size  Algn Flg
     1 (none)               ???              0         0      0 ...
     2 .interp              PROGBITS       792        28      1 .A.
     3 .note.gnu.property   NOTE           824        48      8 .A.
     4 .note.gnu.build-id   NOTE           872        36      4 .A.
     5 .note.ABI-tag        NOTE           908        32      4 .A.
     6 .gnu.hash            ???            944        36      8 .A.
     7 .dynsym              DYNSYM         984       144      8 .A.
     8 .dynstr              STRTAB        1128       136      1 .A.
     9 .gnu.version         ???           1264        12      2 .A.
    10 .gnu.version_r       ???           1280        48      8 .A.
    11 .rela.dyn            RELA          1328       192      8 .A.
    12 .init                PROGBITS      4096        27      4 .AX
    13 .plt                 PROGBITS      4128        16     16 .AX
    14 .plt.got             PROGBITS      4144        16     16 .AX
    15 .text                PROGBITS      4160       248     16 .AX
    16 .fini                PROGBITS      4408        13      4 .AX
    17 .rodata              PROGBITS      8192         4      4 .A.
    18 .eh_frame_hdr        PROGBITS      8196        44      4 .A.
    19 .eh_frame            PROGBITS      8240       148      8 .A.
    20 .init_array          ???          11760         8      8 WA.
    21 .fini_array          ???          11768         8      8 WA.
    22 .dynamic             DYNAMIC      11776       448      8 WA.
    23 .got                 PROGBITS     12224        64      8 WA.
    24 .data                PROGBITS     12288        16      8 WA.
    25 .bss                 NOBITS       12304         8      1 WA.
    26 .comment             PROGBITS     12304        43      1 ...
    27 .symtab              SYMTAB       12352       840      8 ...
    28 .strtab              STRTAB       13192       457      1 ...
    29 .shstrtab            STRTAB       13649       268      1 ...

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
     1 LOAD           0       488 0x08048000 0x08048000       488 4096 R..
     2 LOAD        4096    453520 0x08049000 0x08049000    453520 4096 R.X
     3 LOAD      458752    203059 0x080b8000 0x080b8000    203059 4096 R..
     4 LOAD      662696     13904 0x080eaca8 0x080eaca8     25900 4096 RW.
     5 NOTE         308        68 0x08048134 0x08048134        68    4 R..
     6 TLS       662696        16 0x080eaca8 0x080eaca8        52    4 R..
     7 ???            0         0 0x00000000 0x00000000         0   16 RW.
     8 ???       662696      9048 0x080eaca8 0x080eaca8      9048    1 R..
  Section Headers - Num 29, Size 40, Offset 745384  (sh str ndx 28)
    S# Name                 Type        Offset      Size  Algn Flg
     1 (none)               ???              0         0      0 ...
     2 .note.gnu.build-id   NOTE           308        36      4 .A.
     3 .note.ABI-tag        NOTE           344        32      4 .A.
     4 .rel.plt             REL            376       112      4 .A.
     5 .init                PROGBITS      4096        36      4 .AX
     6 .plt                 PROGBITS      4136       112      8 .AX
     7 .text                PROGBITS      4256    450343     16 .AX
     8 __libc_freeres_fn    PROGBITS    454608      2981     16 .AX
     9 .fini                PROGBITS    457592        24      4 .AX
    10 .rodata              PROGBITS    458752    114752     32 .A.
    11 .eh_frame            PROGBITS    573504     88044      4 .A.
    12 .gcc_except_table    PROGBITS    661548       263      1 .A.
    13 .tdata               PROGBITS    662696        16      4 WA.
    14 .tbss                NOBITS      662712        36      4 WA.
    15 .init_array          ???         662712         4      4 WA.
    16 .fini_array          ???         662716         4      4 WA.
    17 .data.rel.ro         PROGBITS    662720      8948     32 WA.
    18 .got                 PROGBITS    671668        64      4 WA.
    19 .got.plt             PROGBITS    671744        68      4 WA.
    20 .data                PROGBITS    671840      3752     32 WA.
    21 __libc_subfreeres    PROGBITS    675592        36      4 WA.
    22 __libc_IO_vtables    PROGBITS    675648       948     32 WA.
    23 __libc_atexit        PROGBITS    676596         4      4 WA.
    24 .bss                 NOBITS      676600     11972     32 WA.
    25 __libc_freeres_ptrs  NOBITS      676600        16      4 WA.
    26 .comment             PROGBITS    676600        43      1 ...
    27 .symtab              SYMTAB      676644     39200      4 ...
    28 .strtab              STRTAB      715844     29245      1 ...
    29 .shstrtab            STRTAB      745089       295      1 ...

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
     1 LOAD           0      1320 0x00400000 0x00400000      1320 4096 R..
     2 LOAD        4096    615613 0x00401000 0x00401000    615613 4096 R.X
     3 LOAD      622592    164982 0x00498000 0x00498000    164982 4096 R..
     4 LOAD      788400     23264 0x004c17b0 0x004c17b0     46224 4096 RW.
     5 NOTE         624        48 0x00400270 0x00400270        48    8 R..
     6 NOTE         672        68 0x004002a0 0x004002a0        68    4 R..
     7 TLS       788400        32 0x004c17b0 0x004c17b0       104    8 R..
     8 ???          624        48 0x00400270 0x00400270        48    8 R..
     9 ???            0         0 0x00000000 0x00000000         0   16 RW.
    10 ???       788400     14416 0x004c17b0 0x004c17b0     14416    1 R..
  Section Headers - Num 32, Size 64, Offset 898128  (sh str ndx 31)
    S# Name                 Type        Offset      Size  Algn Flg
     1 (none)               ???              0         0      0 ...
     2 .note.gnu.property   NOTE           624        48      8 .A.
     3 .note.gnu.build-id   NOTE           672        36      4 .A.
     4 .note.ABI-tag        NOTE           708        32      4 .A.
     5 .rela.plt            RELA           744       576      8 .A.
     6 .init                PROGBITS      4096        27      4 .AX
     7 .plt                 PROGBITS      4128       384     16 .AX
     8 .text                PROGBITS      4544    609816     64 .AX
     9 __libc_freeres_fn    PROGBITS    614368      5328     16 .AX
    10 .fini                PROGBITS    619696        13      4 .AX
    11 .rodata              PROGBITS    622592    117468     32 .A.
    12 .stapsdt.base        PROGBITS    740060         1      1 .A.
    13 .eh_frame            PROGBITS    740064     47248      8 .A.
    14 .gcc_except_table    PROGBITS    787312       262      1 .A.
    15 .tdata               PROGBITS    788400        32      8 WA.
    16 .tbss                NOBITS      788432        72      8 WA.
    17 .init_array          ???         788432         8      8 WA.
    18 .fini_array          ???         788440         8      8 WA.
    19 .data.rel.ro         PROGBITS    788448     14216     32 WA.
    20 .got                 PROGBITS    802664       152      8 WA.
    21 .got.plt             PROGBITS    802816       216      8 WA.
    22 .data                PROGBITS    803040      6632     32 WA.
    23 __libc_subfreeres    PROGBITS    809672        72      8 WA.
    24 __libc_IO_vtables    PROGBITS    809760      1896     32 WA.
    25 __libc_atexit        PROGBITS    811656         8      8 WA.
    26 .bss                 NOBITS      811664     22912     32 WA.
    27 __libc_freeres_ptrs  NOBITS      811664        32      8 WA.
    28 .comment             PROGBITS    811664        43      1 ...
    29 .note.stapsdt        NOTE        811708      5704      4 ...
    30 .symtab              SYMTAB      817416     50112      8 ...
    31 .strtab              STRTAB      867528     30257      1 ...
    32 .shstrtab            STRTAB      897785       343      1 ...
```