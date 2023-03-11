// auto generated file using mkinstrtab, do not edit manually

struct instruction_encoding_info {
    enum opcode instr;
    enum optype op1type;
    enum optype op2type;
    enum open   op_en;
    char *enc_bytes;
    enum enc_flags;
};

struct instruction_encoding_info compat_instruction_encodings[] = {
    { OC_NOP,  0,      0,      OE_ZO, "NP 90",         }, // One byte no-operation instruction.
    { OC_NOP,  reg16,  0,      OE_M,  "NP 0F 1F /0",   }, // Multi-byte no-operation instruction.
    { OC_NOP,  mem16,  0,      OE_M,  "NP 0F 1F /0",   }, // Multi-byte no-operation instruction.
    { OC_NOP,  reg32,  0,      OE_M,  "NP 0F 1F /0",   }, // Multi-byte no-operation instruction.
    { OC_NOP,  mem32,  0,      OE_M,  "NP 0F 1F /0",   }, // Multi-byte no-operation instruction.
    { OC_MOV,  reg8,   r8,     OE_MR, "88 /r",         }, // Move r8 to r/m8.
    { OC_MOV,  mem8,   r8,     OE_MR, "88 /r",         }, // Move r8 to r/m8.
    { OC_MOV,  reg16,  r16,    OE_MR, "89 /r",         }, // Move r16 to r/m16.
    { OC_MOV,  mem16,  r16,    OE_MR, "89 /r",         }, // Move r16 to r/m16.
    { OC_MOV,  reg32,  r32,    OE_MR, "89 /r",         }, // Move r32 to r/m32.
    { OC_MOV,  mem32,  r32,    OE_MR, "89 /r",         }, // Move r32 to r/m32.
    { OC_MOV,  r8,     reg8,   OE_RM, "8A /r",         }, // Move r/m8 to r8.
    { OC_MOV,  r8,     mem8,   OE_RM, "8A /r",         }, // Move r/m8 to r8.
    { OC_MOV,  r16,    reg16,  OE_RM, "8B /r",         }, // Move r/m16 to r16.
    { OC_MOV,  r16,    mem16,  OE_RM, "8B /r",         }, // Move r/m16 to r16.
    { OC_MOV,  r32,    reg32,  OE_RM, "8B /r",         }, // Move r/m32 to r32.
    { OC_MOV,  r32,    mem32,  OE_RM, "8B /r",         }, // Move r/m32 to r32.
    { OC_MOV,  reg16,  Sreg,   OE_MR, "8C /r",         }, // Move segment register to r/m16.
    { OC_MOV,  mem16,  Sreg,   OE_MR, "8C /r",         }, // Move segment register to r/m16.
    { OC_MOV,  r16,    Sreg,   OE_MR, "8C /r",         }, // Move zero extended 16-bit segment register to r16/r32/m16.
    { OC_MOV,  r32,    Sreg,   OE_MR, "8C /r",         }, // Move zero extended 16-bit segment register to r16/r32/m16.
    { OC_MOV,  m16,    Sreg,   OE_MR, "8C /r",         }, // Move zero extended 16-bit segment register to r16/r32/m16.
    { OC_MOV,  r64,    Sreg,   OE_MR, "REX.W + 8C /r", }, // Move zero extended 16-bit segment register to r64/m16.
    { OC_MOV,  m16,    Sreg,   OE_MR, "REX.W + 8C /r", }, // Move zero extended 16-bit segment register to r64/m16.
    { OC_MOV,  Sreg,   reg162, OE_RM, "8E /r",         }, // Move r/m16 to segment register.
    { OC_MOV,  Sreg,   mem162, OE_RM, "8E /r",         }, // Move r/m16 to segment register.
    { OC_MOV,  Sreg,   reg642, OE_RM, "REX.W + 8E /r", }, // Move lower 16 bits of r/m64 to segment register.
    { OC_MOV,  Sreg,   mem642, OE_RM, "REX.W + 8E /r", }, // Move lower 16 bits of r/m64 to segment register.
    { OC_MOV,  AL,     moffs83,  OE_FD, "A0",            }, // Move byte at (seg:offset) to AL.
    { OC_MOV,  AX,     moffs163,   OE_FD, "A1",            }, // Move word at (seg:offset) to AX.
    { OC_MOV,  EAX,    moffs323,   OE_FD, "A1",            }, // Move doubleword at (seg:offset) to EAX.
    { OC_MOV,  moffs8, AL,     OE_TD, "A2",            }, // Move AL to (seg:offset).
    { OC_MOV,  moffs163,   AX,     OE_TD, "A3",            }, // Move AX to (seg:offset).
    { OC_MOV,  moffs323,   EAX,    OE_TD, "A3",            }, // Move EAX to (seg:offset).
    { OC_MOV,  r8,     imm8,   OE_OI, "B0+ rb ib",     }, // Move imm8 to r8.
    { OC_MOV,  r16,    imm16,  OE_OI, "B8+ rw iw",     }, // Move imm16 to r16.
    { OC_MOV,  r32,    imm32,  OE_OI, "B8+ rd id",     }, // Move imm32 to r32.
    { OC_MOV,  reg8,   imm8,   OE_MI, "C6 /0 ib",      }, // Move imm8 to r/m8.
    { OC_MOV,  mem8,   imm8,   OE_MI, "C6 /0 ib",      }, // Move imm8 to r/m8.
    { OC_MOV,  reg16,  imm16,  OE_MI, "C7 /0 iw",      }, // Move imm16 to r/m16.
    { OC_MOV,  mem16,  imm16,  OE_MI, "C7 /0 iw",      }, // Move imm16 to r/m16.
    { OC_MOV,  reg32,  imm32,  OE_MI, "C7 /0 id",      }, // Move imm32 to r/m32.
    { OC_MOV,  mem32,  imm32,  OE_MI, "C7 /0 id",      }, // Move imm32 to r/m32.
    { OC_PUSH, reg16,  0,      OE_M,  "FF /6",         }, // Push r/m16.
    { OC_PUSH, mem16,  0,      OE_M,  "FF /6",         }, // Push r/m16.
    { OC_PUSH, reg32,  0,      OE_M,  "FF /6",         }, // Push r/m32.
    { OC_PUSH, mem32,  0,      OE_M,  "FF /6",         }, // Push r/m32.
    { OC_PUSH, r16,    0,      OE_O,  "50+rw",         }, // Push r16.
    { OC_PUSH, r32,    0,      OE_O,  "50+rd",         }, // Push r32.
    { OC_PUSH, imm8,   0,      OE_I,  "6A ib",         }, // Push imm8.
    { OC_PUSH, imm16,  0,      OE_I,  "68 iw",         }, // Push imm16.
    { OC_PUSH, imm32,  0,      OE_I,  "68 id",         }, // Push imm32.
    { OC_PUSH, CS,     0,      OE_ZO, "0E",            }, // Push CS.
    { OC_PUSH, SS,     0,      OE_ZO, "16",            }, // Push SS.
    { OC_PUSH, DS,     0,      OE_ZO, "1E",            }, // Push DS.
    { OC_PUSH, ES,     0,      OE_ZO, "06",            }, // Push ES.
    { OC_PUSH, FS,     0,      OE_ZO, "0F A0",         }, // Push FS.
    { OC_PUSH, GS,     0,      OE_ZO, "0F A8",         }, // Push GS.
    { OC_POP,  reg16,  0,      OE_M,  "8F /0",         }, // Pop top of stack into m16; increment stack pointer.
    { OC_POP,  mem16,  0,      OE_M,  "8F /0",         }, // Pop top of stack into m16; increment stack pointer.
    { OC_POP,  reg32,  0,      OE_M,  "8F /0",         }, // Pop top of stack into m32; increment stack pointer.
    { OC_POP,  mem32,  0,      OE_M,  "8F /0",         }, // Pop top of stack into m32; increment stack pointer.
    { OC_POP,  r16,    0,      OE_O,  "58+ rw",        }, // Pop top of stack into r16; increment stack pointer.
    { OC_POP,  r32,    0,      OE_O,  "58+ rd",        }, // Pop top of stack into r32; increment stack pointer.
    { OC_POP,  DS,     0,      OE_ZO, "1F",            }, // Pop top of stack into DS; increment stack pointer.
    { OC_POP,  ES,     0,      OE_ZO, "07",            }, // Pop top of stack into ES; increment stack pointer.
    { OC_POP,  SS,     0,      OE_ZO, "17",            }, // Pop top of stack into SS; increment stack pointer.
    { OC_POP,  FS,     0,      OE_ZO, "0F A1",         }, // Pop top of stack into FS; increment stack pointer by 16 bits.
    { OC_POP,  FS,     0,      OE_ZO, "0F A1",         }, // Pop top of stack into FS; increment stack pointer by 32 bits.
    { OC_POP,  GS,     0,      OE_ZO, "0F A9",         }, // Pop top of stack into GS; increment stack pointer by 16 bits.
    { OC_POP,  GS,     0,      OE_ZO, "0F A9",         }, // Pop top of stack into GS; increment stack pointer by 32 bits.
    { OC_LEA,  r16,m,  0,      OE_RM, "8D /r",         }, // Store effective address for m in register r16.
    { OC_LEA,  r32,m,  0,      OE_RM, "8D /r",         }, // Store effective address for m in register r32.
    { OC_ADD,  AL,     imm8,   OE_I,  "04 ib",         }, // Add imm8 to AL.
    { OC_ADD,  AX,     imm16,  OE_I,  "05 iw",         }, // Add imm16 to AX.
    { OC_ADD,  EAX,    imm32,  OE_I,  "05 id",         }, // Add imm32 to EAX.
    { OC_ADD,  reg8,   imm8,   OE_MI, "80 /0 ib",      }, // Add imm8 to r/m8.
    { OC_ADD,  mem8,   imm8,   OE_MI, "80 /0 ib",      }, // Add imm8 to r/m8.
    { OC_ADD,  reg16,  imm16,  OE_MI, "81 /0 iw",      }, // Add imm16 to r/m16.
    { OC_ADD,  mem16,  imm16,  OE_MI, "81 /0 iw",      }, // Add imm16 to r/m16.
    { OC_ADD,  reg32,  imm32,  OE_MI, "81 /0 id",      }, // Add imm32 to r/m32.
    { OC_ADD,  mem32,  imm32,  OE_MI, "81 /0 id",      }, // Add imm32 to r/m32.
    { OC_ADD,  reg16,  imm8,   OE_MI, "83 /0 ib",      }, // Add sign-extended imm8 to r/m16.
    { OC_ADD,  mem16,  imm8,   OE_MI, "83 /0 ib",      }, // Add sign-extended imm8 to r/m16.
    { OC_ADD,  reg32,  imm8,   OE_MI, "83 /0 ib",      }, // Add sign-extended imm8 to r/m32.
    { OC_ADD,  mem32,  imm8,   OE_MI, "83 /0 ib",      }, // Add sign-extended imm8 to r/m32.
    { OC_ADD,  reg8,   r8,     OE_MR, "00 /r",         }, // Add r8 to r/m8.
    { OC_ADD,  mem8,   r8,     OE_MR, "00 /r",         }, // Add r8 to r/m8.
    { OC_ADD,  reg16,  r16,    OE_MR, "01 /r",         }, // Add r16 to r/m16.
    { OC_ADD,  mem16,  r16,    OE_MR, "01 /r",         }, // Add r16 to r/m16.
    { OC_ADD,  reg32,  r32,    OE_MR, "01 /r",         }, // Add r32 to r/m32.
    { OC_ADD,  mem32,  r32,    OE_MR, "01 /r",         }, // Add r32 to r/m32.
    { OC_ADD,  r8,     reg8,   OE_RM, "02 /r",         }, // Add r/m8 to r8.
    { OC_ADD,  r8,     mem8,   OE_RM, "02 /r",         }, // Add r/m8 to r8.
    { OC_ADD,  r16,    reg16,  OE_RM, "03 /r",         }, // Add r/m16 to r16.
    { OC_ADD,  r16,    mem16,  OE_RM, "03 /r",         }, // Add r/m16 to r16.
    { OC_ADD,  r32,    reg32,  OE_RM, "03 /r",         }, // Add r/m32 to r32.
    { OC_ADD,  r32,    mem32,  OE_RM, "03 /r",         }, // Add r/m32 to r32.
    { OC_SUB,  AL,     imm8,   OE_I,  "2C ib",         }, // Subtract imm8 from AL.
    { OC_SUB,  AX,     imm16,  OE_I,  "2D iw",         }, // Subtract imm16 from AX.
    { OC_SUB,  EAX,    imm32,  OE_I,  "2D id",         }, // Subtract imm32 from EAX.
    { OC_SUB,  reg8,   imm8,   OE_MI, "80 /5 ib",      }, // Subtract imm8 from r/m8.
    { OC_SUB,  mem8,   imm8,   OE_MI, "80 /5 ib",      }, // Subtract imm8 from r/m8.
    { OC_SUB,  reg16,  imm16,  OE_MI, "81 /5 iw",      }, // Subtract imm16 from r/m16.
    { OC_SUB,  mem16,  imm16,  OE_MI, "81 /5 iw",      }, // Subtract imm16 from r/m16.
    { OC_SUB,  reg32,  imm32,  OE_MI, "81 /5 id",      }, // Subtract imm32 from r/m32.
    { OC_SUB,  mem32,  imm32,  OE_MI, "81 /5 id",      }, // Subtract imm32 from r/m32.
    { OC_SUB,  reg16,  imm8,   OE_MI, "83 /5 ib",      }, // Subtract sign-extended imm8 from r/m16.
    { OC_SUB,  mem16,  imm8,   OE_MI, "83 /5 ib",      }, // Subtract sign-extended imm8 from r/m16.
    { OC_SUB,  reg32,  imm8,   OE_MI, "83 /5 ib",      }, // Subtract sign-extended imm8 from r/m32.
    { OC_SUB,  mem32,  imm8,   OE_MI, "83 /5 ib",      }, // Subtract sign-extended imm8 from r/m32.
    { OC_SUB,  reg8,   r8,     OE_MR, "28 /r",         }, // Subtract r8 from r/m8.
    { OC_SUB,  mem8,   r8,     OE_MR, "28 /r",         }, // Subtract r8 from r/m8.
    { OC_SUB,  reg16,  r16,    OE_MR, "29 /r",         }, // Subtract r16 from r/m16.
    { OC_SUB,  mem16,  r16,    OE_MR, "29 /r",         }, // Subtract r16 from r/m16.
    { OC_SUB,  reg32,  r32,    OE_MR, "29 /r",         }, // Subtract r32 from r/m32.
    { OC_SUB,  mem32,  r32,    OE_MR, "29 /r",         }, // Subtract r32 from r/m32.
    { OC_SUB,  r8,     reg8,   OE_RM, "2A /r",         }, // Subtract r/m8 from r8.
    { OC_SUB,  r8,     mem8,   OE_RM, "2A /r",         }, // Subtract r/m8 from r8.
    { OC_SUB,  r16,    reg16,  OE_RM, "2B /r",         }, // Subtract r/m16 from r16.
    { OC_SUB,  r16,    mem16,  OE_RM, "2B /r",         }, // Subtract r/m16 from r16.
    { OC_SUB,  r32,    reg32,  OE_RM, "2B /r",         }, // Subtract r/m32 from r32.
    { OC_SUB,  r32,    mem32,  OE_RM, "2B /r",         }, // Subtract r/m32 from r32.
    { OC_INC,  reg8,   0,      OE_M,  "FE /0",         }, // Increment r/m byte by 1.
    { OC_INC,  mem8,   0,      OE_M,  "FE /0",         }, // Increment r/m byte by 1.
    { OC_INC,  reg16,  0,      OE_M,  "FF /0",         }, // Increment r/m word by 1.
    { OC_INC,  mem16,  0,      OE_M,  "FF /0",         }, // Increment r/m word by 1.
    { OC_INC,  reg32,  0,      OE_M,  "FF /0",         }, // Increment r/m doubleword by 1.
    { OC_INC,  mem32,  0,      OE_M,  "FF /0",         }, // Increment r/m doubleword by 1.
    { OC_INC,  r16,    0,      OE_O,  "40+ rw2",       }, // Increment word register by 1.
    { OC_INC,  r32,    0,      OE_O,  "40+ rd",        }, // Increment doubleword register by 1.
    { OC_DEC,  reg8,   0,      OE_M,  "FE /1",         }, // Decrement r/m8 by 1.
    { OC_DEC,  mem8,   0,      OE_M,  "FE /1",         }, // Decrement r/m8 by 1.
    { OC_DEC,  reg16,  0,      OE_M,  "FF /1",         }, // Decrement r/m16 by 1.
    { OC_DEC,  mem16,  0,      OE_M,  "FF /1",         }, // Decrement r/m16 by 1.
    { OC_DEC,  reg32,  0,      OE_M,  "FF /1",         }, // Decrement r/m32 by 1.
    { OC_DEC,  mem32,  0,      OE_M,  "FF /1",         }, // Decrement r/m32 by 1.
    { OC_DEC,  r16,    0,      OE_O,  "48+rw",         }, // Decrement r16 by 1.
    { OC_DEC,  r32,    0,      OE_O,  "48+rd",         }, // Decrement r32 by 1.
    { OC_IMUL, reg81,  0,      OE_M,  "F6 /5",         }, // AX:= AL ∗ r/m byte.
    { OC_IMUL, mem81,  0,      OE_M,  "F6 /5",         }, // AX:= AL ∗ r/m byte.
    { OC_IMUL, reg16,  0,      OE_M,  "F7 /5",         }, // DX:AX := AX ∗ r/m word.
    { OC_IMUL, mem16,  0,      OE_M,  "F7 /5",         }, // DX:AX := AX ∗ r/m word.
    { OC_IMUL, reg32,  0,      OE_M,  "F7 /5",         }, // EDX:EAX := EAX ∗ r/m32.
    { OC_IMUL, mem32,  0,      OE_M,  "F7 /5",         }, // EDX:EAX := EAX ∗ r/m32.
    { OC_IMUL, r16,    reg16,  OE_RM, "0F AF /r",      }, // word register := word register ∗ r/m16.
    { OC_IMUL, r16,    mem16,  OE_RM, "0F AF /r",      }, // word register := word register ∗ r/m16.
    { OC_IMUL, r32,    reg32,  OE_RM, "0F AF /r",      }, // doubleword register := doubleword register ∗ r/m32.
    { OC_IMUL, r32,    mem32,  OE_RM, "0F AF /r",      }, // doubleword register := doubleword register ∗ r/m32.
    { OC_IMUL, r16,    reg16,, OE_RMI,  "6B /r ib",      }, // word register := r/m16 ∗ sign-extended immediate byte.
    { OC_IMUL, r16,    mem16,, OE_RMI,  "6B /r ib",      }, // word register := r/m16 ∗ sign-extended immediate byte.
    { OC_IMUL, r32,    reg32,, OE_RMI,  "6B /r ib",      }, // doubleword register := r/m32 ∗ signextended immediate byte.
    { OC_IMUL, r32,    mem32,, OE_RMI,  "6B /r ib",      }, // doubleword register := r/m32 ∗ signextended immediate byte.
    { OC_IMUL, r16,    reg16,, OE_RMI,  "69 /r iw",      }, // word register := r/m16 ∗ immediate word.
    { OC_IMUL, r16,    mem16,, OE_RMI,  "69 /r iw",      }, // word register := r/m16 ∗ immediate word.
    { OC_IMUL, r32,    reg32,, OE_RMI,  "69 /r id",      }, // doubleword register := r/m32 ∗ immediate doubleword.
    { OC_IMUL, r32,    mem32,, OE_RMI,  "69 /r id",      }, // doubleword register := r/m32 ∗ immediate doubleword.
    { OC_IDIV, reg8,   0,      OE_M,  "F6 /7",         }, // Signed divide AX by r/m8, with result stored in: AL := Quotient, AH := Remainder.
    { OC_IDIV, mem8,   0,      OE_M,  "F6 /7",         }, // Signed divide AX by r/m8, with result stored in: AL := Quotient, AH := Remainder.
    { OC_IDIV, reg16,  0,      OE_M,  "F7 /7",         }, // Signed divide DX:AX by r/m16, with result stored in AX := Quotient, DX := Remainder.
    { OC_IDIV, mem16,  0,      OE_M,  "F7 /7",         }, // Signed divide DX:AX by r/m16, with result stored in AX := Quotient, DX := Remainder.
    { OC_IDIV, reg32,  0,      OE_M,  "F7 /7",         }, // Signed divide EDX:EAX by r/m32, with result stored in EAX := Quotient, EDX := Remainder.
    { OC_IDIV, mem32,  0,      OE_M,  "F7 /7",         }, // Signed divide EDX:EAX by r/m32, with result stored in EAX := Quotient, EDX := Remainder.
    { OC_AND,  AL,     imm8,   OE_I,  "24 ib",         }, // AL AND imm8.
    { OC_AND,  AX,     imm16,  OE_I,  "25 iw",         }, // AX AND imm16.
    { OC_AND,  EAX,    imm32,  OE_I,  "25 id",         }, // EAX AND imm32.
    { OC_AND,  reg8,   imm8,   OE_MI, "80 /4 ib",      }, // r/m8 AND imm8.
    { OC_AND,  mem8,   imm8,   OE_MI, "80 /4 ib",      }, // r/m8 AND imm8.
    { OC_AND,  reg16,  imm16,  OE_MI, "81 /4 iw",      }, // r/m16 AND imm16.
    { OC_AND,  mem16,  imm16,  OE_MI, "81 /4 iw",      }, // r/m16 AND imm16.
    { OC_AND,  reg32,  imm32,  OE_MI, "81 /4 id",      }, // r/m32 AND imm32.
    { OC_AND,  mem32,  imm32,  OE_MI, "81 /4 id",      }, // r/m32 AND imm32.
    { OC_AND,  reg16,  imm8,   OE_MI, "83 /4 ib",      }, // r/m16 AND imm8 (sign-extended).
    { OC_AND,  mem16,  imm8,   OE_MI, "83 /4 ib",      }, // r/m16 AND imm8 (sign-extended).
    { OC_AND,  reg32,  imm8,   OE_MI, "83 /4 ib",      }, // r/m32 AND imm8 (sign-extended).
    { OC_AND,  mem32,  imm8,   OE_MI, "83 /4 ib",      }, // r/m32 AND imm8 (sign-extended).
    { OC_AND,  reg8,   r8,     OE_MR, "20 /r",         }, // r/m8 AND r8.
    { OC_AND,  mem8,   r8,     OE_MR, "20 /r",         }, // r/m8 AND r8.
    { OC_AND,  reg16,  r16,    OE_MR, "21 /r",         }, // r/m16 AND r16.
    { OC_AND,  mem16,  r16,    OE_MR, "21 /r",         }, // r/m16 AND r16.
    { OC_AND,  reg32,  r32,    OE_MR, "21 /r",         }, // r/m32 AND r32.
    { OC_AND,  mem32,  r32,    OE_MR, "21 /r",         }, // r/m32 AND r32.
    { OC_AND,  r8,     reg8,   OE_RM, "22 /r",         }, // r8 AND r/m8.
    { OC_AND,  r8,     mem8,   OE_RM, "22 /r",         }, // r8 AND r/m8.
    { OC_AND,  r16,    reg16,  OE_RM, "23 /r",         }, // r16 AND r/m16.
    { OC_AND,  r16,    mem16,  OE_RM, "23 /r",         }, // r16 AND r/m16.
    { OC_AND,  r32,    reg32,  OE_RM, "23 /r",         }, // r32 AND r/m32.
    { OC_AND,  r32,    mem32,  OE_RM, "23 /r",         }, // r32 AND r/m32.
    { OC_OR,   AL,     imm8,   OE_I,  "0C ib",         }, // AL OR imm8.
    { OC_OR,   AX,     imm16,  OE_I,  "0D iw",         }, // AX OR imm16.
    { OC_OR,   EAX,    imm32,  OE_I,  "0D id",         }, // EAX OR imm32.
    { OC_OR,   reg8,   imm8,   OE_MI, "80 /1 ib",      }, // r/m8 OR imm8.
    { OC_OR,   mem8,   imm8,   OE_MI, "80 /1 ib",      }, // r/m8 OR imm8.
    { OC_OR,   reg16,  imm16,  OE_MI, "81 /1 iw",      }, // r/m16 OR imm16.
    { OC_OR,   mem16,  imm16,  OE_MI, "81 /1 iw",      }, // r/m16 OR imm16.
    { OC_OR,   reg32,  imm32,  OE_MI, "81 /1 id",      }, // r/m32 OR imm32.
    { OC_OR,   mem32,  imm32,  OE_MI, "81 /1 id",      }, // r/m32 OR imm32.
    { OC_OR,   reg16,  imm8,   OE_MI, "83 /1 ib",      }, // r/m16 OR imm8 (sign-extended).
    { OC_OR,   mem16,  imm8,   OE_MI, "83 /1 ib",      }, // r/m16 OR imm8 (sign-extended).
    { OC_OR,   reg32,  imm8,   OE_MI, "83 /1 ib",      }, // r/m32 OR imm8 (sign-extended).
    { OC_OR,   mem32,  imm8,   OE_MI, "83 /1 ib",      }, // r/m32 OR imm8 (sign-extended).
    { OC_OR,   reg8,   r8,     OE_MR, "08 /r",         }, // r/m8 OR r8.
    { OC_OR,   mem8,   r8,     OE_MR, "08 /r",         }, // r/m8 OR r8.
    { OC_OR,   reg16,  r16,    OE_MR, "09 /r",         }, // r/m16 OR r16.
    { OC_OR,   mem16,  r16,    OE_MR, "09 /r",         }, // r/m16 OR r16.
    { OC_OR,   reg32,  r32,    OE_MR, "09 /r",         }, // r/m32 OR r32.
    { OC_OR,   mem32,  r32,    OE_MR, "09 /r",         }, // r/m32 OR r32.
    { OC_OR,   r8,     reg8,   OE_RM, "0A /r",         }, // r8 OR r/m8.
    { OC_OR,   r8,     mem8,   OE_RM, "0A /r",         }, // r8 OR r/m8.
    { OC_OR,   r16,    reg16,  OE_RM, "0B /r",         }, // r16 OR r/m16.
    { OC_OR,   r16,    mem16,  OE_RM, "0B /r",         }, // r16 OR r/m16.
    { OC_OR,   r32,    reg32,  OE_RM, "0B /r",         }, // r32 OR r/m32.
    { OC_OR,   r32,    mem32,  OE_RM, "0B /r",         }, // r32 OR r/m32.
    { OC_XOR,  AL,     imm8,   OE_I,  "34 ib",         }, // AL XOR imm8.
    { OC_XOR,  AX,     imm16,  OE_I,  "35 iw",         }, // AX XOR imm16.
    { OC_XOR,  EAX,    imm32,  OE_I,  "35 id",         }, // EAX XOR imm32.
    { OC_XOR,  reg8,   imm8,   OE_MI, "80 /6 ib",      }, // r/m8 XOR imm8.
    { OC_XOR,  mem8,   imm8,   OE_MI, "80 /6 ib",      }, // r/m8 XOR imm8.
    { OC_XOR,  reg16,  imm16,  OE_MI, "81 /6 iw",      }, // r/m16 XOR imm16.
    { OC_XOR,  mem16,  imm16,  OE_MI, "81 /6 iw",      }, // r/m16 XOR imm16.
    { OC_XOR,  reg32,  imm32,  OE_MI, "81 /6 id",      }, // r/m32 XOR imm32.
    { OC_XOR,  mem32,  imm32,  OE_MI, "81 /6 id",      }, // r/m32 XOR imm32.
    { OC_XOR,  reg16,  imm8,   OE_MI, "83 /6 ib",      }, // r/m16 XOR imm8 (sign-extended).
    { OC_XOR,  mem16,  imm8,   OE_MI, "83 /6 ib",      }, // r/m16 XOR imm8 (sign-extended).
    { OC_XOR,  reg32,  imm8,   OE_MI, "83 /6 ib",      }, // r/m32 XOR imm8 (sign-extended).
    { OC_XOR,  mem32,  imm8,   OE_MI, "83 /6 ib",      }, // r/m32 XOR imm8 (sign-extended).
    { OC_XOR,  reg8,   r8,     OE_MR, "30 /r",         }, // r/m8 XOR r8.
    { OC_XOR,  mem8,   r8,     OE_MR, "30 /r",         }, // r/m8 XOR r8.
    { OC_XOR,  reg16,  r16,    OE_MR, "31 /r",         }, // r/m16 XOR r16.
    { OC_XOR,  mem16,  r16,    OE_MR, "31 /r",         }, // r/m16 XOR r16.
    { OC_XOR,  reg32,  r32,    OE_MR, "31 /r",         }, // r/m32 XOR r32.
    { OC_XOR,  mem32,  r32,    OE_MR, "31 /r",         }, // r/m32 XOR r32.
    { OC_XOR,  r8,     reg8,   OE_RM, "32 /r",         }, // r8 XOR r/m8.
    { OC_XOR,  r8,     mem8,   OE_RM, "32 /r",         }, // r8 XOR r/m8.
    { OC_XOR,  r16,    reg16,  OE_RM, "33 /r",         }, // r16 XOR r/m16.
    { OC_XOR,  r16,    mem16,  OE_RM, "33 /r",         }, // r16 XOR r/m16.
    { OC_XOR,  r32,    reg32,  OE_RM, "33 /r",         }, // r32 XOR r/m32.
    { OC_XOR,  r32,    mem32,  OE_RM, "33 /r",         }, // r32 XOR r/m32.
    { OC_NOT,  reg8,   0,      OE_M,  "F6 /2",         }, // Reverse each bit of r/m8.
    { OC_NOT,  mem8,   0,      OE_M,  "F6 /2",         }, // Reverse each bit of r/m8.
    { OC_NOT,  reg16,  0,      OE_M,  "F7 /2",         }, // Reverse each bit of r/m16.
    { OC_NOT,  mem16,  0,      OE_M,  "F7 /2",         }, // Reverse each bit of r/m16.
    { OC_NOT,  reg32,  0,      OE_M,  "F7 /2",         }, // Reverse each bit of r/m32.
    { OC_NOT,  mem32,  0,      OE_M,  "F7 /2",         }, // Reverse each bit of r/m32.
    { OC_NEG,  reg8,   0,      OE_M,  "F6 /3",         }, // Two's complement negate r/m8.
    { OC_NEG,  mem8,   0,      OE_M,  "F6 /3",         }, // Two's complement negate r/m8.
    { OC_NEG,  reg16,  0,      OE_M,  "F7 /3",         }, // Two's complement negate r/m16.
    { OC_NEG,  mem16,  0,      OE_M,  "F7 /3",         }, // Two's complement negate r/m16.
    { OC_NEG,  reg32,  0,      OE_M,  "F7 /3",         }, // Two's complement negate r/m32.
    { OC_NEG,  mem32,  0,      OE_M,  "F7 /3",         }, // Two's complement negate r/m32.
    { OC_SHL,  reg8,   1,      OE_M1, "D0 /4",         }, // Multiply r/m8 by 2, once.
    { OC_SHL,  mem8,   1,      OE_M1, "D0 /4",         }, // Multiply r/m8 by 2, once.
    { OC_SHL,  reg8,   CL,     OE_MC, "D2 /4",         }, // Multiply r/m8 by 2, CL times.
    { OC_SHL,  mem8,   CL,     OE_MC, "D2 /4",         }, // Multiply r/m8 by 2, CL times.
    { OC_SHL,  reg8,   imm8,   OE_MI, "C0 /4 ib",      }, // Multiply r/m8 by 2, imm8 times.
    { OC_SHL,  mem8,   imm8,   OE_MI, "C0 /4 ib",      }, // Multiply r/m8 by 2, imm8 times.
    { OC_SHL,  reg16,1,  0,      OE_M1, "D1 /4",         }, // Multiply r/m16 by 2, once.
    { OC_SHL,  mem16,1,  0,      OE_M1, "D1 /4",         }, // Multiply r/m16 by 2, once.
    { OC_SHL,  reg16,  CL,     OE_MC, "D3 /4",         }, // Multiply r/m16 by 2, CL times.
    { OC_SHL,  mem16,  CL,     OE_MC, "D3 /4",         }, // Multiply r/m16 by 2, CL times.
    { OC_SHL,  reg16,  imm8,   OE_MI, "C1 /4 ib",      }, // Multiply r/m16 by 2, imm8 times.
    { OC_SHL,  mem16,  imm8,   OE_MI, "C1 /4 ib",      }, // Multiply r/m16 by 2, imm8 times.
    { OC_SHL,  reg32,1,  0,      OE_M1, "D1 /4",         }, // Multiply r/m32 by 2, once.
    { OC_SHL,  mem32,1,  0,      OE_M1, "D1 /4",         }, // Multiply r/m32 by 2, once.
    { OC_SHL,  reg32,  CL,     OE_MC, "D3 /4",         }, // Multiply r/m32 by 2, CL times.
    { OC_SHL,  mem32,  CL,     OE_MC, "D3 /4",         }, // Multiply r/m32 by 2, CL times.
    { OC_SHL,  reg32,  imm8,   OE_MI, "C1 /4 ib",      }, // Multiply r/m32 by 2, imm8 times.
    { OC_SHL,  mem32,  imm8,   OE_MI, "C1 /4 ib",      }, // Multiply r/m32 by 2, imm8 times.
    { OC_SHR,  reg8,1, 0,      OE_M1, "D0 /5",         }, // Unsigned divide r/m8 by 2, once.
    { OC_SHR,  mem8,1, 0,      OE_M1, "D0 /5",         }, // Unsigned divide r/m8 by 2, once.
    { OC_SHR,  reg8,   CL,     OE_MC, "D2 /5",         }, // Unsigned divide r/m8 by 2, CL times.
    { OC_SHR,  mem8,   CL,     OE_MC, "D2 /5",         }, // Unsigned divide r/m8 by 2, CL times.
    { OC_SHR,  reg8,   imm8,   OE_MI, "C0 /5 ib",      }, // Unsigned divide r/m8 by 2, imm8 times.
    { OC_SHR,  mem8,   imm8,   OE_MI, "C0 /5 ib",      }, // Unsigned divide r/m8 by 2, imm8 times.
    { OC_SHR,  reg16,  1,      OE_M1, "D1 /5",         }, // Unsigned divide r/m16 by 2, once.
    { OC_SHR,  mem16,  1,      OE_M1, "D1 /5",         }, // Unsigned divide r/m16 by 2, once.
    { OC_SHR,  reg16,  CL,     OE_MC, "D3 /5",         }, // Unsigned divide r/m16 by 2, CL times
    { OC_SHR,  mem16,  CL,     OE_MC, "D3 /5",         }, // Unsigned divide r/m16 by 2, CL times
    { OC_SHR,  reg16,  imm8,   OE_MI, "C1 /5 ib",      }, // Unsigned divide r/m16 by 2, imm8 times.
    { OC_SHR,  mem16,  imm8,   OE_MI, "C1 /5 ib",      }, // Unsigned divide r/m16 by 2, imm8 times.
    { OC_SHR,  reg32,  1,      OE_M1, "D1 /5",         }, // Unsigned divide r/m32 by 2, once.
    { OC_SHR,  mem32,  1,      OE_M1, "D1 /5",         }, // Unsigned divide r/m32 by 2, once.
    { OC_SHR,  reg32,  CL,     OE_MC, "D3 /5",         }, // Unsigned divide r/m32 by 2, CL times.
    { OC_SHR,  mem32,  CL,     OE_MC, "D3 /5",         }, // Unsigned divide r/m32 by 2, CL times.
    { OC_SHR,  reg32,  imm8,   OE_MI, "C1 /5 ib",      }, // Unsigned divide r/m32 by 2, imm8 times.
    { OC_SHR,  mem32,  imm8,   OE_MI, "C1 /5 ib",      }, // Unsigned divide r/m32 by 2, imm8 times.
    { OC_JMP,  rel8,   0,      OE_D,  "EB cb",         }, // Jump short, RIP = RIP + 8-bit displacement sign extended to 64-bits.
    { OC_JMP,  rel16,  0,      OE_D,  "E9 cw",         }, // Jump near, relative, displacement relative to next instruction. Not supported in 64-bit mode.
    { OC_JMP,  rel32,  0,      OE_D,  "E9 cd",         }, // Jump near, relative, RIP = RIP + 32-bit displacement sign extended to 64-bits.
    { OC_JMP,  reg16,  0,      OE_M,  "FF /4",         }, // Jump near, absolute indirect, address = zeroextended r/m16. Not supported in 64-bit mode.
    { OC_JMP,  mem16,  0,      OE_M,  "FF /4",         }, // Jump near, absolute indirect, address = zeroextended r/m16. Not supported in 64-bit mode.
    { OC_JMP,  reg32,  0,      OE_M,  "FF /4",         }, // Jump near, absolute indirect, address given in r/m32. Not supported in 64-bit mode.
    { OC_JMP,  mem32,  0,      OE_M,  "FF /4",         }, // Jump near, absolute indirect, address given in r/m32. Not supported in 64-bit mode.
    { OC_JMP,  ptr16:16,   0,      OE_S,  "EA cd",         }, // Jump far, absolute, address given in operand.
    { OC_JMP,  ptr16:32,   0,      OE_S,  "EA cp",         }, // Jump far, absolute, address given in operand.
    { OC_JMP,  m16:16, 0,      OE_M,  "FF /5",         }, // Jump far, absolute indirect, address given in m16:16.
    { OC_JMP,  m16:32, 0,      OE_M,  "FF /5",         }, // Jump far, absolute indirect, address given in m16:32.
    { OC_CMP,  AL,     imm8,   OE_I,  "3C ib",         }, // Compare imm8 with AL.
    { OC_CMP,  AX,     imm16,  OE_I,  "3D iw",         }, // Compare imm16 with AX.
    { OC_CMP,  EAX,    imm32,  OE_I,  "3D id",         }, // Compare imm32 with EAX.
    { OC_CMP,  reg8,   imm8,   OE_MI, "80 /7 ib",      }, // Compare imm8 with r/m8.
    { OC_CMP,  mem8,   imm8,   OE_MI, "80 /7 ib",      }, // Compare imm8 with r/m8.
    { OC_CMP,  reg16,  imm16,  OE_MI, "81 /7 iw",      }, // Compare imm16 with r/m16.
    { OC_CMP,  mem16,  imm16,  OE_MI, "81 /7 iw",      }, // Compare imm16 with r/m16.
    { OC_CMP,  reg32,  imm32,  OE_MI, "81 /7 id",      }, // Compare imm32 with r/m32.
    { OC_CMP,  mem32,  imm32,  OE_MI, "81 /7 id",      }, // Compare imm32 with r/m32.
    { OC_CMP,  reg16,  imm8,   OE_MI, "83 /7 ib",      }, // Compare imm8 with r/m16.
    { OC_CMP,  mem16,  imm8,   OE_MI, "83 /7 ib",      }, // Compare imm8 with r/m16.
    { OC_CMP,  reg32,  imm8,   OE_MI, "83 /7 ib",      }, // Compare imm8 with r/m32.
    { OC_CMP,  mem32,  imm8,   OE_MI, "83 /7 ib",      }, // Compare imm8 with r/m32.
    { OC_CMP,  reg8,   r8,     OE_MR, "38 /r",         }, // Compare r8 with r/m8.
    { OC_CMP,  mem8,   r8,     OE_MR, "38 /r",         }, // Compare r8 with r/m8.
    { OC_CMP,  reg16,  r16,    OE_MR, "39 /r",         }, // Compare r16 with r/m16.
    { OC_CMP,  mem16,  r16,    OE_MR, "39 /r",         }, // Compare r16 with r/m16.
    { OC_CMP,  reg32,  r32,    OE_MR, "39 /r",         }, // Compare r32 with r/m32.
    { OC_CMP,  mem32,  r32,    OE_MR, "39 /r",         }, // Compare r32 with r/m32.
    { OC_CMP,  r8,     reg8,   OE_RM, "3A /r",         }, // Compare r/m8 with r8.
    { OC_CMP,  r8,     mem8,   OE_RM, "3A /r",         }, // Compare r/m8 with r8.
    { OC_CMP,  r16,    reg16,  OE_RM, "3B /r",         }, // Compare r/m16 with r16.
    { OC_CMP,  r16,    mem16,  OE_RM, "3B /r",         }, // Compare r/m16 with r16.
    { OC_CMP,  r32,    reg32,  OE_RM, "3B /r",         }, // Compare r/m32 with r32.
    { OC_CMP,  r32,    mem32,  OE_RM, "3B /r",         }, // Compare r/m32 with r32.
    { OC_JA,   rel8,   0,      OE_D,  "77 cb",         }, // Jump short if above (CF=0 and ZF=0).
    { OC_JA,   rel16,  0,      OE_D,  "0F 87 cw",      }, // Jump near if above (CF=0 and ZF=0). Not supported in 64-bit mode.
    { OC_JA,   rel32,  0,      OE_D,  "0F 87 cd",      }, // Jump near if above (CF=0 and ZF=0).
    { OC_JAE,  rel8,   0,      OE_D,  "73 cb",         }, // Jump short if above or equal (CF=0).
    { OC_JAE,  rel16,  0,      OE_D,  "0F 83 cw",      }, // Jump near if above or equal (CF=0). Not supported in 64-bit mode.
    { OC_JAE,  rel32,  0,      OE_D,  "0F 83 cd",      }, // Jump near if above or equal (CF=0).
    { OC_JB,   rel8,   0,      OE_D,  "72 cb",         }, // Jump short if below (CF=1).
    { OC_JB,   rel16,  0,      OE_D,  "0F 82 cw",      }, // Jump near if below (CF=1). Not supported in 64-bit mode.
    { OC_JB,   rel32,  0,      OE_D,  "0F 82 cd",      }, // Jump near if below (CF=1).
    { OC_JBE,  rel8,   0,      OE_D,  "76 cb",         }, // Jump short if below or equal (CF=1 or ZF=1).
    { OC_JBE,  rel16,  0,      OE_D,  "0F 86 cw",      }, // Jump near if below or equal (CF=1 or ZF=1). Not supported in 64-bit mode.
    { OC_JBE,  rel32,  0,      OE_D,  "0F 86 cd",      }, // Jump near if below or equal (CF=1 or ZF=1).
    { OC_JC,   rel8,   0,      OE_D,  "72 cb",         }, // Jump short if carry (CF=1).
    { OC_JC,   rel16,  0,      OE_D,  "0F 82 cw",      }, // Jump near if carry (CF=1). Not supported in 64-bit mode.
    { OC_JC,   rel32,  0,      OE_D,  "0F 82 cd",      }, // Jump near if carry (CF=1).
    { OC_JCXZ, rel8,   0,      OE_D,  "E3 cb",         }, // Jump short if CX register is 0.
    { OC_JECXZ,  rel8,   0,      OE_D,  "E3 cb",         }, // Jump short if ECX register is 0.
    { OC_JE,   rel8,   0,      OE_D,  "74 cb",         }, // Jump short if equal (ZF=1).
    { OC_JE,   rel16,  0,      OE_D,  "0F 84 cw",      }, // Jump near if equal (ZF=1). Not supported in 64-bit mode.
    { OC_JE,   rel32,  0,      OE_D,  "0F 84 cd",      }, // Jump near if equal (ZF=1).
    { OC_JZ,   rel16,  0,      OE_D,  "0F 84 cw",      }, // Jump near if 0 (ZF=1). Not supported in 64-bit mode.
    { OC_JZ,   rel32,  0,      OE_D,  "0F 84 cd",      }, // Jump near if 0 (ZF=1).
    { OC_JG,   rel8,   0,      OE_D,  "7F cb",         }, // Jump short if greater (ZF=0 and SF=OF).
    { OC_JG,   rel16,  0,      OE_D,  "0F 8F cw",      }, // Jump near if greater (ZF=0 and SF=OF). Not supported in 64-bit mode.
    { OC_JG,   rel32,  0,      OE_D,  "0F 8F cd",      }, // Jump near if greater (ZF=0 and SF=OF).
    { OC_JGE,  rel8,   0,      OE_D,  "7D cb",         }, // Jump short if greater or equal (SF=OF).
    { OC_JGE,  rel16,  0,      OE_D,  "0F 8D cw",      }, // Jump near if greater or equal (SF=OF). Not supported in 64-bit mode.
    { OC_JGE,  rel32,  0,      OE_D,  "0F 8D cd",      }, // Jump near if greater or equal (SF=OF).
    { OC_JL,   rel8,   0,      OE_D,  "7C cb",         }, // Jump short if less (SF≠ OF).
    { OC_JL,   rel16,  0,      OE_D,  "0F 8C cw",      }, // Jump near if less (SF≠ OF). Not supported in 64-bit mode.
    { OC_JL,   rel32,  0,      OE_D,  "0F 8C cd",      }, // Jump near if less (SF≠ OF).
    { OC_JLE,  rel8,   0,      OE_D,  "7E cb",         }, // Jump short if less or equal (ZF=1 or SF≠ OF).
    { OC_JLE,  rel16,  0,      OE_D,  "0F 8E cw",      }, // Jump near if less or equal (ZF=1 or SF≠ OF). Not supported in 64-bit mode.
    { OC_JLE,  rel32,  0,      OE_D,  "0F 8E cd",      }, // Jump near if less or equal (ZF=1 or SF≠ OF).
    { OC_JNA,  rel8,   0,      OE_D,  "76 cb",         }, // Jump short if not above (CF=1 or ZF=1).
    { OC_JNA,  rel16,  0,      OE_D,  "0F 86 cw",      }, // Jump near if not above (CF=1 or ZF=1). Not supported in 64-bit mode.
    { OC_JNA,  rel32,  0,      OE_D,  "0F 86 cd",      }, // Jump near if not above (CF=1 or ZF=1).
    { OC_JNAE, rel8,   0,      OE_D,  "72 cb",         }, // Jump short if not above or equal (CF=1).
    { OC_JNAE, rel16,  0,      OE_D,  "0F 82 cw",      }, // Jump near if not above or equal (CF=1). Not supported in 64-bit mode.
    { OC_JNAE, rel32,  0,      OE_D,  "0F 82 cd",      }, // Jump near if not above or equal (CF=1).
    { OC_JNB,  rel8,   0,      OE_D,  "73 cb",         }, // Jump short if not below (CF=0).
    { OC_JNB,  rel16,  0,      OE_D,  "0F 83 cw",      }, // Jump near if not below (CF=0). Not supported in 64-bit mode.
    { OC_JNB,  rel32,  0,      OE_D,  "0F 83 cd",      }, // Jump near if not below (CF=0).
    { OC_JNBE, rel8,   0,      OE_D,  "77 cb",         }, // Jump short if not below or equal (CF=0 and ZF=0).
    { OC_JNBE, rel16,  0,      OE_D,  "0F 87 cw",      }, // Jump near if not below or equal (CF=0 and ZF=0). Not supported in 64-bit mode.
    { OC_JNBE, rel32,  0,      OE_D,  "0F 87 cd",      }, // Jump near if not below or equal (CF=0 and ZF=0).
    { OC_JNC,  rel8,   0,      OE_D,  "73 cb",         }, // Jump short if not carry (CF=0).
    { OC_JNC,  rel16,  0,      OE_D,  "0F 83 cw",      }, // Jump near if not carry (CF=0). Not supported in 64-bit mode.
    { OC_JNC,  rel32,  0,      OE_D,  "0F 83 cd",      }, // Jump near if not carry (CF=0).
    { OC_JNE,  rel8,   0,      OE_D,  "75 cb",         }, // Jump short if not equal (ZF=0).
    { OC_JNE,  rel16,  0,      OE_D,  "0F 85 cw",      }, // Jump near if not equal (ZF=0). Not supported in 64-bit mode.
    { OC_JNE,  rel32,  0,      OE_D,  "0F 85 cd",      }, // Jump near if not equal (ZF=0).
    { OC_JNG,  rel8,   0,      OE_D,  "7E cb",         }, // Jump short if not greater (ZF=1 or SF≠ OF).
    { OC_JNG,  rel16,  0,      OE_D,  "0F 8E cw",      }, // Jump near if not greater (ZF=1 or SF≠ OF). Not supported in 64-bit mode.
    { OC_JNG,  rel32,  0,      OE_D,  "0F 8E cd",      }, // Jump near if not greater (ZF=1 or SF≠ OF).
    { OC_JNGE, rel8,   0,      OE_D,  "7C cb",         }, // Jump short if not greater or equal (SF≠ OF).
    { OC_JNGE, rel16,  0,      OE_D,  "0F 8C cw",      }, // Jump near if not greater or equal (SF≠ OF). Not supported in 64-bit mode.
    { OC_JNGE, rel32,  0,      OE_D,  "0F 8C cd",      }, // Jump near if not greater or equal (SF≠ OF).
    { OC_JNL,  rel8,   0,      OE_D,  "7D cb",         }, // Jump short if not less (SF=OF).
    { OC_JNL,  rel16,  0,      OE_D,  "0F 8D cw",      }, // Jump near if not less (SF=OF). Not supported in 64-bit mode.
    { OC_JNL,  rel32,  0,      OE_D,  "0F 8D cd",      }, // Jump near if not less (SF=OF).
    { OC_JNLE, rel8,   0,      OE_D,  "7F cb",         }, // Jump short if not less or equal (ZF=0 and SF=OF).
    { OC_JNLE, rel16,  0,      OE_D,  "0F 8F cw",      }, // Jump near if not less or equal (ZF=0 and SF=OF). Not supported in 64-bit mode.
    { OC_JNLE, rel32,  0,      OE_D,  "0F 8F cd",      }, // Jump near if not less or equal (ZF=0 and SF=OF).
    { OC_JNO,  rel8,   0,      OE_D,  "71 cb",         }, // Jump short if not overflow (OF=0).
    { OC_JNO,  rel16,  0,      OE_D,  "0F 81 cw",      }, // Jump near if not overflow (OF=0). Not supported in 64-bit mode.
    { OC_JNO,  rel32,  0,      OE_D,  "0F 81 cd",      }, // Jump near if not overflow (OF=0).
    { OC_JNP,  rel8,   0,      OE_D,  "7B cb",         }, // Jump short if not parity (PF=0).
    { OC_JNP,  rel16,  0,      OE_D,  "0F 8B cw",      }, // Jump near if not parity (PF=0). Not supported in 64-bit mode.
    { OC_JNP,  rel32,  0,      OE_D,  "0F 8B cd",      }, // Jump near if not parity (PF=0).
    { OC_JNS,  rel8,   0,      OE_D,  "79 cb",         }, // Jump short if not sign (SF=0).
    { OC_JNS,  rel16,  0,      OE_D,  "0F 89 cw",      }, // Jump near if not sign (SF=0). Not supported in 64-bit mode.
    { OC_JNS,  rel32,  0,      OE_D,  "0F 89 cd",      }, // Jump near if not sign (SF=0).
    { OC_JNZ,  rel8,   0,      OE_D,  "75 cb",         }, // Jump short if not zero (ZF=0).
    { OC_JNZ,  rel16,  0,      OE_D,  "0F 85 cw",      }, // Jump near if not zero (ZF=0). Not supported in 64-bit mode.
    { OC_JNZ,  rel32,  0,      OE_D,  "0F 85 cd",      }, // Jump near if not zero (ZF=0).
    { OC_JO,   rel8,   0,      OE_D,  "70 cb",         }, // Jump short if overflow (OF=1).
    { OC_JO,   rel16,  0,      OE_D,  "0F 80 cw",      }, // Jump near if overflow (OF=1). Not supported in 64-bit mode.
    { OC_JO,   rel32,  0,      OE_D,  "0F 80 cd",      }, // Jump near if overflow (OF=1).
    { OC_JP,   rel8,   0,      OE_D,  "7A cb",         }, // Jump short if parity (PF=1).
    { OC_JP,   rel16,  0,      OE_D,  "0F 8A cw",      }, // Jump near if parity (PF=1). Not supported in 64-bit mode.
    { OC_JP,   rel32,  0,      OE_D,  "0F 8A cd",      }, // Jump near if parity (PF=1).
    { OC_JPE,  rel8,   0,      OE_D,  "7A cb",         }, // Jump short if parity even (PF=1).
    { OC_JPE,  rel16,  0,      OE_D,  "0F 8A cw",      }, // Jump near if parity even (PF=1). Not supported in 64-bit mode.
    { OC_JPE,  rel32,  0,      OE_D,  "0F 8A cd",      }, // Jump near if parity even (PF=1).
    { OC_JPO,  rel8,   0,      OE_D,  "7B cb",         }, // Jump short if parity odd (PF=0).
    { OC_JPO,  rel16,  0,      OE_D,  "0F 8B cw",      }, // Jump near if parity odd (PF=0). Not supported in 64-bit mode.
    { OC_JPO,  rel32,  0,      OE_D,  "0F 8B cd",      }, // Jump near if parity odd (PF=0).
    { OC_JS,   rel8,   0,      OE_D,  "78 cb",         }, // Jump short if sign (SF=1).
    { OC_JS,   rel16,  0,      OE_D,  "0F 88 cw",      }, // Jump near if sign (SF=1). Not supported in 64-bit mode.
    { OC_JS,   rel32,  0,      OE_D,  "0F 88 cd",      }, // Jump near if sign (SF=1).
    { OC_JZ,   rel8,   0,      OE_D,  "74 cb",         }, // Jump short if zero (ZF = 1).
    { OC_JZ,   rel16,  0,      OE_D,  "0F 84 cw",      }, // Jump near if 0 (ZF=1). Not supported in 64-bit mode.
    { OC_JZ,   rel32,  0,      OE_D,  "0F 84 cd",      }, // Jump near if 0 (ZF=1).
    { OC_CALL, rel16,  0,      OE_D,  "E8 cw",         }, // Call near, relative, displacement relative to next instruction.
    { OC_CALL, rel32,  0,      OE_D,  "E8 cd",         }, // Call near, relative, displacement relative to next instruction. 32-bit displacement sign extended to 64-bits in 64-bit mode.
    { OC_CALL, reg16,  0,      OE_M,  "FF /2",         }, // Call near, absolute indirect, address given in r/m16.
    { OC_CALL, mem16,  0,      OE_M,  "FF /2",         }, // Call near, absolute indirect, address given in r/m16.
    { OC_CALL, reg32,  0,      OE_M,  "FF /2",         }, // Call near, absolute indirect, address given in r/m32.
    { OC_CALL, mem32,  0,      OE_M,  "FF /2",         }, // Call near, absolute indirect, address given in r/m32.
    { OC_CALL, ptr16:16,   0,      OE_D,  "9A cd",         }, // Call far, absolute, address given in operand.
    { OC_CALL, ptr16:32,   0,      OE_D,  "9A cp",         }, // Call far, absolute, address given in operand.
    { OC_CALL, m16:16, 0,      OE_M,  "FF /3",         }, // Call far, absolute indirect address given in m16:16. In 32-bit mode: if selector points to a gate, then RIP = 32-bit zero extended displacement taken from gate; else RIP = zero extended 16-bit offset from far pointer referenced in the instruction.
    { OC_CALL, m16:32, 0,      OE_M,  "FF /3",         }, // In 64-bit mode: If selector points to a gate, then RIP = 64-bit displacement taken from gate; else RIP = zero extended 32-bit offset from far pointer referenced in the instruction.
    { OC_RET,  0,      0,      OE_ZO, "C3",            }, // Near return to calling procedure.
    { OC_RET,  0,      0,      OE_ZO, "CB",            }, // Far return to calling procedure.
    { OC_RET,  imm16,  0,      OE_I,  "C2 iw",         }, // Near return to calling procedure and pop imm16 bytes from stack.
    { OC_RET,  imm16,  0,      OE_I,  "CA iw",         }, // Far return to calling procedure and pop imm16 bytes from stack.
    { OC_INT,  imm8,   0,      OE_I,  "CD ib",         }, // Generate software interrupt with vector specified by immediate byte.
};

struct instruction_encoding_info x64_instruction_encodings[] = {
    { OC_NOP,  0,      0,      OE_ZO, "NP 90",         }, // One byte no-operation instruction.
    { OC_NOP,  reg16,  0,      OE_M,  "NP 0F 1F /0",   }, // Multi-byte no-operation instruction.
    { OC_NOP,  mem16,  0,      OE_M,  "NP 0F 1F /0",   }, // Multi-byte no-operation instruction.
    { OC_NOP,  reg32,  0,      OE_M,  "NP 0F 1F /0",   }, // Multi-byte no-operation instruction.
    { OC_NOP,  mem32,  0,      OE_M,  "NP 0F 1F /0",   }, // Multi-byte no-operation instruction.
    { OC_MOV,  reg8,   r8,     OE_MR, "88 /r",         }, // Move r8 to r/m8.
    { OC_MOV,  mem8,   r8,     OE_MR, "88 /r",         }, // Move r8 to r/m8.
    { OC_MOV,  reg81,  r81,    OE_MR, "REX + 88 /r",   }, // Move r8 to r/m8.
    { OC_MOV,  mem81,  r81,    OE_MR, "REX + 88 /r",   }, // Move r8 to r/m8.
    { OC_MOV,  reg16,  r16,    OE_MR, "89 /r",         }, // Move r16 to r/m16.
    { OC_MOV,  mem16,  r16,    OE_MR, "89 /r",         }, // Move r16 to r/m16.
    { OC_MOV,  reg32,  r32,    OE_MR, "89 /r",         }, // Move r32 to r/m32.
    { OC_MOV,  mem32,  r32,    OE_MR, "89 /r",         }, // Move r32 to r/m32.
    { OC_MOV,  reg64,  r64,    OE_MR, "REX.W + 89 /r", }, // Move r64 to r/m64.
    { OC_MOV,  mem64,  r64,    OE_MR, "REX.W + 89 /r", }, // Move r64 to r/m64.
    { OC_MOV,  r8,     reg8,   OE_RM, "8A /r",         }, // Move r/m8 to r8.
    { OC_MOV,  r8,     mem8,   OE_RM, "8A /r",         }, // Move r/m8 to r8.
    { OC_MOV,  r81,    reg81,  OE_RM, "REX + 8A /r",   }, // Move r/m8 to r8.
    { OC_MOV,  r81,    mem81,  OE_RM, "REX + 8A /r",   }, // Move r/m8 to r8.
    { OC_MOV,  r16,    reg16,  OE_RM, "8B /r",         }, // Move r/m16 to r16.
    { OC_MOV,  r16,    mem16,  OE_RM, "8B /r",         }, // Move r/m16 to r16.
    { OC_MOV,  r32,    reg32,  OE_RM, "8B /r",         }, // Move r/m32 to r32.
    { OC_MOV,  r32,    mem32,  OE_RM, "8B /r",         }, // Move r/m32 to r32.
    { OC_MOV,  r64,    reg64,  OE_RM, "REX.W + 8B /r", }, // Move r/m64 to r64.
    { OC_MOV,  r64,    mem64,  OE_RM, "REX.W + 8B /r", }, // Move r/m64 to r64.
    { OC_MOV,  reg16,  Sreg,   OE_MR, "8C /r",         }, // Move segment register to r/m16.
    { OC_MOV,  mem16,  Sreg,   OE_MR, "8C /r",         }, // Move segment register to r/m16.
    { OC_MOV,  r16,    Sreg,   OE_MR, "8C /r",         }, // Move zero extended 16-bit segment register to r16/r32/m16.
    { OC_MOV,  r32,    Sreg,   OE_MR, "8C /r",         }, // Move zero extended 16-bit segment register to r16/r32/m16.
    { OC_MOV,  m16,    Sreg,   OE_MR, "8C /r",         }, // Move zero extended 16-bit segment register to r16/r32/m16.
    { OC_MOV,  r64,    Sreg,   OE_MR, "REX.W + 8C /r", }, // Move zero extended 16-bit segment register to r64/m16.
    { OC_MOV,  m16,    Sreg,   OE_MR, "REX.W + 8C /r", }, // Move zero extended 16-bit segment register to r64/m16.
    { OC_MOV,  Sreg,   reg162, OE_RM, "8E /r",         }, // Move r/m16 to segment register.
    { OC_MOV,  Sreg,   mem162, OE_RM, "8E /r",         }, // Move r/m16 to segment register.
    { OC_MOV,  Sreg,   reg642, OE_RM, "REX.W + 8E /r", }, // Move lower 16 bits of r/m64 to segment register.
    { OC_MOV,  Sreg,   mem642, OE_RM, "REX.W + 8E /r", }, // Move lower 16 bits of r/m64 to segment register.
    { OC_MOV,  AL,     moffs83,  OE_FD, "A0",            }, // Move byte at (seg:offset) to AL.
    { OC_MOV,  AL,     moffs83,  OE_FD, "REX.W + A0",    }, // Move byte at (offset) to AL.
    { OC_MOV,  AX,     moffs163,   OE_FD, "A1",            }, // Move word at (seg:offset) to AX.
    { OC_MOV,  EAX,    moffs323,   OE_FD, "A1",            }, // Move doubleword at (seg:offset) to EAX.
    { OC_MOV,  RAX,    moffs643,   OE_FD, "REX.W + A1",    }, // Move quadword at (offset) to RAX.
    { OC_MOV,  moffs8, AL,     OE_TD, "A2",            }, // Move AL to (seg:offset).
    { OC_MOV,  moffs81,  AL,     OE_TD, "REX.W + A2",    }, // Move AL to (offset).
    { OC_MOV,  moffs163,   AX,     OE_TD, "A3",            }, // Move AX to (seg:offset).
    { OC_MOV,  moffs323,   EAX,    OE_TD, "A3",            }, // Move EAX to (seg:offset).
    { OC_MOV,  moffs643,   RAX,    OE_TD, "REX.W + A3",    }, // Move RAX to (offset).
    { OC_MOV,  r8,     imm8,   OE_OI, "B0+ rb ib",     }, // Move imm8 to r8.
    { OC_MOV,  r81,    imm8,   OE_OI, "REX + B0+ rb ib", }, // Move imm8 to r8.
    { OC_MOV,  r16,    imm16,  OE_OI, "B8+ rw iw",     }, // Move imm16 to r16.
    { OC_MOV,  r32,    imm32,  OE_OI, "B8+ rd id",     }, // Move imm32 to r32.
    { OC_MOV,  r64,    imm64,  OE_OI, "REX.W + B8+ rd io",   }, // Move imm64 to r64.
    { OC_MOV,  reg8,   imm8,   OE_MI, "C6 /0 ib",      }, // Move imm8 to r/m8.
    { OC_MOV,  mem8,   imm8,   OE_MI, "C6 /0 ib",      }, // Move imm8 to r/m8.
    { OC_MOV,  reg81,  imm8,   OE_MI, "REX + C6 /0 ib",}, // Move imm8 to r/m8.
    { OC_MOV,  mem81,  imm8,   OE_MI, "REX + C6 /0 ib",}, // Move imm8 to r/m8.
    { OC_MOV,  reg16,  imm16,  OE_MI, "C7 /0 iw",      }, // Move imm16 to r/m16.
    { OC_MOV,  mem16,  imm16,  OE_MI, "C7 /0 iw",      }, // Move imm16 to r/m16.
    { OC_MOV,  reg32,  imm32,  OE_MI, "C7 /0 id",      }, // Move imm32 to r/m32.
    { OC_MOV,  mem32,  imm32,  OE_MI, "C7 /0 id",      }, // Move imm32 to r/m32.
    { OC_MOV,  reg64,  imm32,  OE_MI, "REX.W + C7 /0 id",  }, // Move imm32 sign extended to 64-bits to r/m64
    { OC_MOV,  mem64,  imm32,  OE_MI, "REX.W + C7 /0 id",  }, // Move imm32 sign extended to 64-bits to r/m64
    { OC_PUSH, reg16,  0,      OE_M,  "FF /6",         }, // Push r/m16.
    { OC_PUSH, mem16,  0,      OE_M,  "FF /6",         }, // Push r/m16.
    { OC_PUSH, reg64,  0,      OE_M,  "FF /6",         }, // Push r/m64.
    { OC_PUSH, mem64,  0,      OE_M,  "FF /6",         }, // Push r/m64.
    { OC_PUSH, r16,    0,      OE_O,  "50+rw",         }, // Push r16.
    { OC_PUSH, r64,    0,      OE_O,  "50+rd",         }, // Push r64.
    { OC_PUSH, imm8,   0,      OE_I,  "6A ib",         }, // Push imm8.
    { OC_PUSH, imm16,  0,      OE_I,  "68 iw",         }, // Push imm16.
    { OC_PUSH, imm32,  0,      OE_I,  "68 id",         }, // Push imm32.
    { OC_PUSH, FS,     0,      OE_ZO, "0F A0",         }, // Push FS.
    { OC_PUSH, GS,     0,      OE_ZO, "0F A8",         }, // Push GS.
    { OC_POP,  reg16,  0,      OE_M,  "8F /0",         }, // Pop top of stack into m16; increment stack pointer.
    { OC_POP,  mem16,  0,      OE_M,  "8F /0",         }, // Pop top of stack into m16; increment stack pointer.
    { OC_POP,  reg64,  0,      OE_M,  "8F /0",         }, // Pop top of stack into m64; increment stack pointer. Cannot encode 32-bit operand size.
    { OC_POP,  mem64,  0,      OE_M,  "8F /0",         }, // Pop top of stack into m64; increment stack pointer. Cannot encode 32-bit operand size.
    { OC_POP,  r16,    0,      OE_O,  "58+ rw",        }, // Pop top of stack into r16; increment stack pointer.
    { OC_POP,  r64,    0,      OE_O,  "58+ rd",        }, // Pop top of stack into r64; increment stack pointer. Cannot encode 32-bit operand size.
    { OC_POP,  FS,     0,      OE_ZO, "0F A1",         }, // Pop top of stack into FS; increment stack pointer by 16 bits.
    { OC_POP,  FS,     0,      OE_ZO, "0F A1",         }, // Pop top of stack into FS; increment stack pointer by 64 bits.
    { OC_POP,  GS,     0,      OE_ZO, "0F A9",         }, // Pop top of stack into GS; increment stack pointer by 16 bits.
    { OC_POP,  GS,     0,      OE_ZO, "0F A9",         }, // Pop top of stack into GS; increment stack pointer by 64 bits.
    { OC_LEA,  r16,m,  0,      OE_RM, "8D /r",         }, // Store effective address for m in register r16.
    { OC_LEA,  r32,m,  0,      OE_RM, "8D /r",         }, // Store effective address for m in register r32.
    { OC_LEA,  r64,m,  0,      OE_RM, "REX.W + 8D /r", }, // Store effective address for m in register r64.
    { OC_ADD,  AL,     imm8,   OE_I,  "04 ib",         }, // Add imm8 to AL.
    { OC_ADD,  AX,     imm16,  OE_I,  "05 iw",         }, // Add imm16 to AX.
    { OC_ADD,  EAX,    imm32,  OE_I,  "05 id",         }, // Add imm32 to EAX.
    { OC_ADD,  RAX,    imm32,  OE_I,  "REX.W + 05 id", }, // Add imm32 sign-extended to 64-bits to RAX.
    { OC_ADD,  reg8,   imm8,   OE_MI, "80 /0 ib",      }, // Add imm8 to r/m8.
    { OC_ADD,  mem8,   imm8,   OE_MI, "80 /0 ib",      }, // Add imm8 to r/m8.
    { OC_ADD,  reg8*,  imm8,   OE_MI, "REX + 80 /0 ib",}, // Add sign-extended imm8 to r/m8.
    { OC_ADD,  mem8*,  imm8,   OE_MI, "REX + 80 /0 ib",}, // Add sign-extended imm8 to r/m8.
    { OC_ADD,  reg16,  imm16,  OE_MI, "81 /0 iw",      }, // Add imm16 to r/m16.
    { OC_ADD,  mem16,  imm16,  OE_MI, "81 /0 iw",      }, // Add imm16 to r/m16.
    { OC_ADD,  reg32,  imm32,  OE_MI, "81 /0 id",      }, // Add imm32 to r/m32.
    { OC_ADD,  mem32,  imm32,  OE_MI, "81 /0 id",      }, // Add imm32 to r/m32.
    { OC_ADD,  reg64,  imm32,  OE_MI, "REX.W + 81 /0 id",  }, // Add imm32 sign-extended to 64-bits to r/m64.
    { OC_ADD,  mem64,  imm32,  OE_MI, "REX.W + 81 /0 id",  }, // Add imm32 sign-extended to 64-bits to r/m64.
    { OC_ADD,  reg16,  imm8,   OE_MI, "83 /0 ib",      }, // Add sign-extended imm8 to r/m16.
    { OC_ADD,  mem16,  imm8,   OE_MI, "83 /0 ib",      }, // Add sign-extended imm8 to r/m16.
    { OC_ADD,  reg32,  imm8,   OE_MI, "83 /0 ib",      }, // Add sign-extended imm8 to r/m32.
    { OC_ADD,  mem32,  imm8,   OE_MI, "83 /0 ib",      }, // Add sign-extended imm8 to r/m32.
    { OC_ADD,  reg64,  imm8,   OE_MI, "REX.W + 83 /0 ib",  }, // Add sign-extended imm8 to r/m64.
    { OC_ADD,  mem64,  imm8,   OE_MI, "REX.W + 83 /0 ib",  }, // Add sign-extended imm8 to r/m64.
    { OC_ADD,  reg8,   r8,     OE_MR, "00 /r",         }, // Add r8 to r/m8.
    { OC_ADD,  mem8,   r8,     OE_MR, "00 /r",         }, // Add r8 to r/m8.
    { OC_ADD,  reg8*,  r8*,    OE_MR, "REX + 00 /r",   }, // Add r8 to r/m8.
    { OC_ADD,  mem8*,  r8*,    OE_MR, "REX + 00 /r",   }, // Add r8 to r/m8.
    { OC_ADD,  reg16,  r16,    OE_MR, "01 /r",         }, // Add r16 to r/m16.
    { OC_ADD,  mem16,  r16,    OE_MR, "01 /r",         }, // Add r16 to r/m16.
    { OC_ADD,  reg32,  r32,    OE_MR, "01 /r",         }, // Add r32 to r/m32.
    { OC_ADD,  mem32,  r32,    OE_MR, "01 /r",         }, // Add r32 to r/m32.
    { OC_ADD,  reg64,  r64,    OE_MR, "REX.W + 01 /r", }, // Add r64 to r/m64.
    { OC_ADD,  mem64,  r64,    OE_MR, "REX.W + 01 /r", }, // Add r64 to r/m64.
    { OC_ADD,  r8,     reg8,   OE_RM, "02 /r",         }, // Add r/m8 to r8.
    { OC_ADD,  r8,     mem8,   OE_RM, "02 /r",         }, // Add r/m8 to r8.
    { OC_ADD,  r8*,    reg8*,  OE_RM, "REX + 02 /r",   }, // Add r/m8 to r8.
    { OC_ADD,  r8*,    mem8*,  OE_RM, "REX + 02 /r",   }, // Add r/m8 to r8.
    { OC_ADD,  r16,    reg16,  OE_RM, "03 /r",         }, // Add r/m16 to r16.
    { OC_ADD,  r16,    mem16,  OE_RM, "03 /r",         }, // Add r/m16 to r16.
    { OC_ADD,  r32,    reg32,  OE_RM, "03 /r",         }, // Add r/m32 to r32.
    { OC_ADD,  r32,    mem32,  OE_RM, "03 /r",         }, // Add r/m32 to r32.
    { OC_ADD,  r64,    reg64,  OE_RM, "REX.W + 03 /r", }, // Add r/m64 to r64.
    { OC_ADD,  r64,    mem64,  OE_RM, "REX.W + 03 /r", }, // Add r/m64 to r64.
    { OC_SUB,  AL,     imm8,   OE_I,  "2C ib",         }, // Subtract imm8 from AL.
    { OC_SUB,  AX,     imm16,  OE_I,  "2D iw",         }, // Subtract imm16 from AX.
    { OC_SUB,  EAX,    imm32,  OE_I,  "2D id",         }, // Subtract imm32 from EAX.
    { OC_SUB,  RAX,    imm32,  OE_I,  "REX.W + 2D id", }, // Subtract imm32 sign-extended to 64-bits from RAX.
    { OC_SUB,  reg8,   imm8,   OE_MI, "80 /5 ib",      }, // Subtract imm8 from r/m8.
    { OC_SUB,  mem8,   imm8,   OE_MI, "80 /5 ib",      }, // Subtract imm8 from r/m8.
    { OC_SUB,  reg81,  imm8,   OE_MI, "REX + 80 /5 ib",}, // Subtract imm8 from r/m8.
    { OC_SUB,  mem81,  imm8,   OE_MI, "REX + 80 /5 ib",}, // Subtract imm8 from r/m8.
    { OC_SUB,  reg16,  imm16,  OE_MI, "81 /5 iw",      }, // Subtract imm16 from r/m16.
    { OC_SUB,  mem16,  imm16,  OE_MI, "81 /5 iw",      }, // Subtract imm16 from r/m16.
    { OC_SUB,  reg32,  imm32,  OE_MI, "81 /5 id",      }, // Subtract imm32 from r/m32.
    { OC_SUB,  mem32,  imm32,  OE_MI, "81 /5 id",      }, // Subtract imm32 from r/m32.
    { OC_SUB,  reg64,  imm32,  OE_MI, "REX.W + 81 /5 id",  }, // Subtract imm32 sign-extended to 64-bits from r/m64.
    { OC_SUB,  mem64,  imm32,  OE_MI, "REX.W + 81 /5 id",  }, // Subtract imm32 sign-extended to 64-bits from r/m64.
    { OC_SUB,  reg16,  imm8,   OE_MI, "83 /5 ib",      }, // Subtract sign-extended imm8 from r/m16.
    { OC_SUB,  mem16,  imm8,   OE_MI, "83 /5 ib",      }, // Subtract sign-extended imm8 from r/m16.
    { OC_SUB,  reg32,  imm8,   OE_MI, "83 /5 ib",      }, // Subtract sign-extended imm8 from r/m32.
    { OC_SUB,  mem32,  imm8,   OE_MI, "83 /5 ib",      }, // Subtract sign-extended imm8 from r/m32.
    { OC_SUB,  reg64,  imm8,   OE_MI, "REX.W + 83 /5 ib",  }, // Subtract sign-extended imm8 from r/m64.
    { OC_SUB,  mem64,  imm8,   OE_MI, "REX.W + 83 /5 ib",  }, // Subtract sign-extended imm8 from r/m64.
    { OC_SUB,  reg8,   r8,     OE_MR, "28 /r",         }, // Subtract r8 from r/m8.
    { OC_SUB,  mem8,   r8,     OE_MR, "28 /r",         }, // Subtract r8 from r/m8.
    { OC_SUB,  reg81,  r81,    OE_MR, "REX + 28 /r",   }, // Subtract r8 from r/m8.
    { OC_SUB,  mem81,  r81,    OE_MR, "REX + 28 /r",   }, // Subtract r8 from r/m8.
    { OC_SUB,  reg16,  r16,    OE_MR, "29 /r",         }, // Subtract r16 from r/m16.
    { OC_SUB,  mem16,  r16,    OE_MR, "29 /r",         }, // Subtract r16 from r/m16.
    { OC_SUB,  reg32,  r32,    OE_MR, "29 /r",         }, // Subtract r32 from r/m32.
    { OC_SUB,  mem32,  r32,    OE_MR, "29 /r",         }, // Subtract r32 from r/m32.
    { OC_SUB,  reg64,  r64,    OE_MR, "REX.W + 29 /r", }, // Subtract r64 from r/m64.
    { OC_SUB,  mem64,  r64,    OE_MR, "REX.W + 29 /r", }, // Subtract r64 from r/m64.
    { OC_SUB,  r8,     reg8,   OE_RM, "2A /r",         }, // Subtract r/m8 from r8.
    { OC_SUB,  r8,     mem8,   OE_RM, "2A /r",         }, // Subtract r/m8 from r8.
    { OC_SUB,  r81,    reg81,  OE_RM, "REX + 2A /r",   }, // Subtract r/m8 from r8.
    { OC_SUB,  r81,    mem81,  OE_RM, "REX + 2A /r",   }, // Subtract r/m8 from r8.
    { OC_SUB,  r16,    reg16,  OE_RM, "2B /r",         }, // Subtract r/m16 from r16.
    { OC_SUB,  r16,    mem16,  OE_RM, "2B /r",         }, // Subtract r/m16 from r16.
    { OC_SUB,  r32,    reg32,  OE_RM, "2B /r",         }, // Subtract r/m32 from r32.
    { OC_SUB,  r32,    mem32,  OE_RM, "2B /r",         }, // Subtract r/m32 from r32.
    { OC_SUB,  r64,    reg64,  OE_RM, "REX.W + 2B /r", }, // Subtract r/m64 from r64.
    { OC_SUB,  r64,    mem64,  OE_RM, "REX.W + 2B /r", }, // Subtract r/m64 from r64.
    { OC_INC,  reg8,   0,      OE_M,  "FE /0",         }, // Increment r/m byte by 1.
    { OC_INC,  mem8,   0,      OE_M,  "FE /0",         }, // Increment r/m byte by 1.
    { OC_INC,  reg81,  0,      OE_M,  "REX + FE /0",   }, // Increment r/m byte by 1.
    { OC_INC,  mem81,  0,      OE_M,  "REX + FE /0",   }, // Increment r/m byte by 1.
    { OC_INC,  reg16,  0,      OE_M,  "FF /0",         }, // Increment r/m word by 1.
    { OC_INC,  mem16,  0,      OE_M,  "FF /0",         }, // Increment r/m word by 1.
    { OC_INC,  reg32,  0,      OE_M,  "FF /0",         }, // Increment r/m doubleword by 1.
    { OC_INC,  mem32,  0,      OE_M,  "FF /0",         }, // Increment r/m doubleword by 1.
    { OC_INC,  reg64,  0,      OE_M,  "REX.W + FF /0", }, // Increment r/m quadword by 1.
    { OC_INC,  mem64,  0,      OE_M,  "REX.W + FF /0", }, // Increment r/m quadword by 1.
    { OC_DEC,  reg8,   0,      OE_M,  "FE /1",         }, // Decrement r/m8 by 1.
    { OC_DEC,  mem8,   0,      OE_M,  "FE /1",         }, // Decrement r/m8 by 1.
    { OC_DEC,  reg8*,  0,      OE_M,  "REX + FE /1",   }, // Decrement r/m8 by 1.
    { OC_DEC,  mem8*,  0,      OE_M,  "REX + FE /1",   }, // Decrement r/m8 by 1.
    { OC_DEC,  reg16,  0,      OE_M,  "FF /1",         }, // Decrement r/m16 by 1.
    { OC_DEC,  mem16,  0,      OE_M,  "FF /1",         }, // Decrement r/m16 by 1.
    { OC_DEC,  reg32,  0,      OE_M,  "FF /1",         }, // Decrement r/m32 by 1.
    { OC_DEC,  mem32,  0,      OE_M,  "FF /1",         }, // Decrement r/m32 by 1.
    { OC_DEC,  reg64,  0,      OE_M,  "REX.W + FF /1", }, // Decrement r/m64 by 1.
    { OC_DEC,  mem64,  0,      OE_M,  "REX.W + FF /1", }, // Decrement r/m64 by 1.
    { OC_IMUL, reg81,  0,      OE_M,  "F6 /5",         }, // AX:= AL ∗ r/m byte.
    { OC_IMUL, mem81,  0,      OE_M,  "F6 /5",         }, // AX:= AL ∗ r/m byte.
    { OC_IMUL, reg16,  0,      OE_M,  "F7 /5",         }, // DX:AX := AX ∗ r/m word.
    { OC_IMUL, mem16,  0,      OE_M,  "F7 /5",         }, // DX:AX := AX ∗ r/m word.
    { OC_IMUL, reg32,  0,      OE_M,  "F7 /5",         }, // EDX:EAX := EAX ∗ r/m32.
    { OC_IMUL, mem32,  0,      OE_M,  "F7 /5",         }, // EDX:EAX := EAX ∗ r/m32.
    { OC_IMUL, reg64,  0,      OE_M,  "REX.W + F7 /5", }, // RDX:RAX := RAX ∗ r/m64.
    { OC_IMUL, mem64,  0,      OE_M,  "REX.W + F7 /5", }, // RDX:RAX := RAX ∗ r/m64.
    { OC_IMUL, r16,    reg16,  OE_RM, "0F AF /r",      }, // word register := word register ∗ r/m16.
    { OC_IMUL, r16,    mem16,  OE_RM, "0F AF /r",      }, // word register := word register ∗ r/m16.
    { OC_IMUL, r32,    reg32,  OE_RM, "0F AF /r",      }, // doubleword register := doubleword register ∗ r/m32.
    { OC_IMUL, r32,    mem32,  OE_RM, "0F AF /r",      }, // doubleword register := doubleword register ∗ r/m32.
    { OC_IMUL, r64,    reg64,  OE_RM, "REX.W + 0F AF /r",  }, // Quadword register := Quadword register ∗ r/m64.
    { OC_IMUL, r64,    mem64,  OE_RM, "REX.W + 0F AF /r",  }, // Quadword register := Quadword register ∗ r/m64.
    { OC_IMUL, r16,    reg16,, OE_RMI,  "6B /r ib",      }, // word register := r/m16 ∗ sign-extended immediate byte.
    { OC_IMUL, r16,    mem16,, OE_RMI,  "6B /r ib",      }, // word register := r/m16 ∗ sign-extended immediate byte.
    { OC_IMUL, r32,    reg32,, OE_RMI,  "6B /r ib",      }, // doubleword register := r/m32 ∗ signextended immediate byte.
    { OC_IMUL, r32,    mem32,, OE_RMI,  "6B /r ib",      }, // doubleword register := r/m32 ∗ signextended immediate byte.
    { OC_IMUL, r64,    reg64,, OE_RMI,  "REX.W + 6B /r ib",  }, // Quadword register := r/m64 ∗ sign-extended immediate byte.
    { OC_IMUL, r64,    mem64,, OE_RMI,  "REX.W + 6B /r ib",  }, // Quadword register := r/m64 ∗ sign-extended immediate byte.
    { OC_IMUL, r16,    reg16,, OE_RMI,  "69 /r iw",      }, // word register := r/m16 ∗ immediate word.
    { OC_IMUL, r16,    mem16,, OE_RMI,  "69 /r iw",      }, // word register := r/m16 ∗ immediate word.
    { OC_IMUL, r32,    reg32,, OE_RMI,  "69 /r id",      }, // doubleword register := r/m32 ∗ immediate doubleword.
    { OC_IMUL, r32,    mem32,, OE_RMI,  "69 /r id",      }, // doubleword register := r/m32 ∗ immediate doubleword.
    { OC_IMUL, r64,    reg64,, OE_RMI,  "REX.W + 69 /r id",  }, // Quadword register := r/m64 ∗ immediate doubleword.
    { OC_IMUL, r64,    mem64,, OE_RMI,  "REX.W + 69 /r id",  }, // Quadword register := r/m64 ∗ immediate doubleword.
    { OC_IDIV, reg8,   0,      OE_M,  "F6 /7",         }, // Signed divide AX by r/m8, with result stored in: AL := Quotient, AH := Remainder.
    { OC_IDIV, mem8,   0,      OE_M,  "F6 /7",         }, // Signed divide AX by r/m8, with result stored in: AL := Quotient, AH := Remainder.
    { OC_IDIV, reg81,  0,      OE_M,  "REX + F6 /7",   }, // Signed divide AX by r/m8, with result stored in AL := Quotient, AH := Remainder.
    { OC_IDIV, mem81,  0,      OE_M,  "REX + F6 /7",   }, // Signed divide AX by r/m8, with result stored in AL := Quotient, AH := Remainder.
    { OC_IDIV, reg16,  0,      OE_M,  "F7 /7",         }, // Signed divide DX:AX by r/m16, with result stored in AX := Quotient, DX := Remainder.
    { OC_IDIV, mem16,  0,      OE_M,  "F7 /7",         }, // Signed divide DX:AX by r/m16, with result stored in AX := Quotient, DX := Remainder.
    { OC_IDIV, reg32,  0,      OE_M,  "F7 /7",         }, // Signed divide EDX:EAX by r/m32, with result stored in EAX := Quotient, EDX := Remainder.
    { OC_IDIV, mem32,  0,      OE_M,  "F7 /7",         }, // Signed divide EDX:EAX by r/m32, with result stored in EAX := Quotient, EDX := Remainder.
    { OC_IDIV, reg64,  0,      OE_M,  "REX.W + F7 /7", }, // Signed divide RDX:RAX by r/m64, with result stored in RAX := Quotient, RDX := Remainder.
    { OC_IDIV, mem64,  0,      OE_M,  "REX.W + F7 /7", }, // Signed divide RDX:RAX by r/m64, with result stored in RAX := Quotient, RDX := Remainder.
    { OC_AND,  AL,     imm8,   OE_I,  "24 ib",         }, // AL AND imm8.
    { OC_AND,  AX,     imm16,  OE_I,  "25 iw",         }, // AX AND imm16.
    { OC_AND,  EAX,    imm32,  OE_I,  "25 id",         }, // EAX AND imm32.
    { OC_AND,  RAX,    imm32,  OE_I,  "REX.W + 25 id", }, // RAX AND imm32 sign-extended to 64-bits.
    { OC_AND,  reg8,   imm8,   OE_MI, "80 /4 ib",      }, // r/m8 AND imm8.
    { OC_AND,  mem8,   imm8,   OE_MI, "80 /4 ib",      }, // r/m8 AND imm8.
    { OC_AND,  reg8*,  imm8,   OE_MI, "REX + 80 /4 ib",}, // r/m8 AND imm8.
    { OC_AND,  mem8*,  imm8,   OE_MI, "REX + 80 /4 ib",}, // r/m8 AND imm8.
    { OC_AND,  reg16,  imm16,  OE_MI, "81 /4 iw",      }, // r/m16 AND imm16.
    { OC_AND,  mem16,  imm16,  OE_MI, "81 /4 iw",      }, // r/m16 AND imm16.
    { OC_AND,  reg32,  imm32,  OE_MI, "81 /4 id",      }, // r/m32 AND imm32.
    { OC_AND,  mem32,  imm32,  OE_MI, "81 /4 id",      }, // r/m32 AND imm32.
    { OC_AND,  reg64,  imm32,  OE_MI, "REX.W + 81 /4 id",  }, // r/m64 AND imm32 sign extended to 64-bits.
    { OC_AND,  mem64,  imm32,  OE_MI, "REX.W + 81 /4 id",  }, // r/m64 AND imm32 sign extended to 64-bits.
    { OC_AND,  reg16,  imm8,   OE_MI, "83 /4 ib",      }, // r/m16 AND imm8 (sign-extended).
    { OC_AND,  mem16,  imm8,   OE_MI, "83 /4 ib",      }, // r/m16 AND imm8 (sign-extended).
    { OC_AND,  reg32,  imm8,   OE_MI, "83 /4 ib",      }, // r/m32 AND imm8 (sign-extended).
    { OC_AND,  mem32,  imm8,   OE_MI, "83 /4 ib",      }, // r/m32 AND imm8 (sign-extended).
    { OC_AND,  reg64,  imm8,   OE_MI, "REX.W + 83 /4 ib",  }, // r/m64 AND imm8 (sign-extended).
    { OC_AND,  mem64,  imm8,   OE_MI, "REX.W + 83 /4 ib",  }, // r/m64 AND imm8 (sign-extended).
    { OC_AND,  reg8,   r8,     OE_MR, "20 /r",         }, // r/m8 AND r8.
    { OC_AND,  mem8,   r8,     OE_MR, "20 /r",         }, // r/m8 AND r8.
    { OC_AND,  reg8*,  r8*,    OE_MR, "REX + 20 /r",   }, // r/m64 AND r8 (sign-extended).
    { OC_AND,  mem8*,  r8*,    OE_MR, "REX + 20 /r",   }, // r/m64 AND r8 (sign-extended).
    { OC_AND,  reg16,  r16,    OE_MR, "21 /r",         }, // r/m16 AND r16.
    { OC_AND,  mem16,  r16,    OE_MR, "21 /r",         }, // r/m16 AND r16.
    { OC_AND,  reg32,  r32,    OE_MR, "21 /r",         }, // r/m32 AND r32.
    { OC_AND,  mem32,  r32,    OE_MR, "21 /r",         }, // r/m32 AND r32.
    { OC_AND,  reg64,  r64,    OE_MR, "REX.W + 21 /r", }, // r/m64 AND r32.
    { OC_AND,  mem64,  r64,    OE_MR, "REX.W + 21 /r", }, // r/m64 AND r32.
    { OC_AND,  r8,     reg8,   OE_RM, "22 /r",         }, // r8 AND r/m8.
    { OC_AND,  r8,     mem8,   OE_RM, "22 /r",         }, // r8 AND r/m8.
    { OC_AND,  r8*,    reg8*,  OE_RM, "REX + 22 /r",   }, // r/m64 AND r8 (sign-extended).
    { OC_AND,  r8*,    mem8*,  OE_RM, "REX + 22 /r",   }, // r/m64 AND r8 (sign-extended).
    { OC_AND,  r16,    reg16,  OE_RM, "23 /r",         }, // r16 AND r/m16.
    { OC_AND,  r16,    mem16,  OE_RM, "23 /r",         }, // r16 AND r/m16.
    { OC_AND,  r32,    reg32,  OE_RM, "23 /r",         }, // r32 AND r/m32.
    { OC_AND,  r32,    mem32,  OE_RM, "23 /r",         }, // r32 AND r/m32.
    { OC_AND,  r64,    reg64,  OE_RM, "REX.W + 23 /r", }, // r64 AND r/m64.
    { OC_AND,  r64,    mem64,  OE_RM, "REX.W + 23 /r", }, // r64 AND r/m64.
    { OC_OR,   AL,     imm8,   OE_I,  "0C ib",         }, // AL OR imm8.
    { OC_OR,   AX,     imm16,  OE_I,  "0D iw",         }, // AX OR imm16.
    { OC_OR,   EAX,    imm32,  OE_I,  "0D id",         }, // EAX OR imm32.
    { OC_OR,   RAX,    imm32,  OE_I,  "REX.W + 0D id", }, // RAX OR imm32 (sign-extended).
    { OC_OR,   reg8,   imm8,   OE_MI, "80 /1 ib",      }, // r/m8 OR imm8.
    { OC_OR,   mem8,   imm8,   OE_MI, "80 /1 ib",      }, // r/m8 OR imm8.
    { OC_OR,   reg81,  imm8,   OE_MI, "REX + 80 /1 ib",}, // r/m8 OR imm8.
    { OC_OR,   mem81,  imm8,   OE_MI, "REX + 80 /1 ib",}, // r/m8 OR imm8.
    { OC_OR,   reg16,  imm16,  OE_MI, "81 /1 iw",      }, // r/m16 OR imm16.
    { OC_OR,   mem16,  imm16,  OE_MI, "81 /1 iw",      }, // r/m16 OR imm16.
    { OC_OR,   reg32,  imm32,  OE_MI, "81 /1 id",      }, // r/m32 OR imm32.
    { OC_OR,   mem32,  imm32,  OE_MI, "81 /1 id",      }, // r/m32 OR imm32.
    { OC_OR,   reg64,  imm32,  OE_MI, "REX.W + 81 /1 id",  }, // r/m64 OR imm32 (sign-extended).
    { OC_OR,   mem64,  imm32,  OE_MI, "REX.W + 81 /1 id",  }, // r/m64 OR imm32 (sign-extended).
    { OC_OR,   reg16,  imm8,   OE_MI, "83 /1 ib",      }, // r/m16 OR imm8 (sign-extended).
    { OC_OR,   mem16,  imm8,   OE_MI, "83 /1 ib",      }, // r/m16 OR imm8 (sign-extended).
    { OC_OR,   reg32,  imm8,   OE_MI, "83 /1 ib",      }, // r/m32 OR imm8 (sign-extended).
    { OC_OR,   mem32,  imm8,   OE_MI, "83 /1 ib",      }, // r/m32 OR imm8 (sign-extended).
    { OC_OR,   reg64,  imm8,   OE_MI, "REX.W + 83 /1 ib",  }, // r/m64 OR imm8 (sign-extended).
    { OC_OR,   mem64,  imm8,   OE_MI, "REX.W + 83 /1 ib",  }, // r/m64 OR imm8 (sign-extended).
    { OC_OR,   reg8,   r8,     OE_MR, "08 /r",         }, // r/m8 OR r8.
    { OC_OR,   mem8,   r8,     OE_MR, "08 /r",         }, // r/m8 OR r8.
    { OC_OR,   reg81,  r81,    OE_MR, "REX + 08 /r",   }, // r/m8 OR r8.
    { OC_OR,   mem81,  r81,    OE_MR, "REX + 08 /r",   }, // r/m8 OR r8.
    { OC_OR,   reg16,  r16,    OE_MR, "09 /r",         }, // r/m16 OR r16.
    { OC_OR,   mem16,  r16,    OE_MR, "09 /r",         }, // r/m16 OR r16.
    { OC_OR,   reg32,  r32,    OE_MR, "09 /r",         }, // r/m32 OR r32.
    { OC_OR,   mem32,  r32,    OE_MR, "09 /r",         }, // r/m32 OR r32.
    { OC_OR,   reg64,  r64,    OE_MR, "REX.W + 09 /r", }, // r/m64 OR r64.
    { OC_OR,   mem64,  r64,    OE_MR, "REX.W + 09 /r", }, // r/m64 OR r64.
    { OC_OR,   r8,     reg8,   OE_RM, "0A /r",         }, // r8 OR r/m8.
    { OC_OR,   r8,     mem8,   OE_RM, "0A /r",         }, // r8 OR r/m8.
    { OC_OR,   r81,    reg81,  OE_RM, "REX + 0A /r",   }, // r8 OR r/m8.
    { OC_OR,   r81,    mem81,  OE_RM, "REX + 0A /r",   }, // r8 OR r/m8.
    { OC_OR,   r16,    reg16,  OE_RM, "0B /r",         }, // r16 OR r/m16.
    { OC_OR,   r16,    mem16,  OE_RM, "0B /r",         }, // r16 OR r/m16.
    { OC_OR,   r32,    reg32,  OE_RM, "0B /r",         }, // r32 OR r/m32.
    { OC_OR,   r32,    mem32,  OE_RM, "0B /r",         }, // r32 OR r/m32.
    { OC_OR,   r64,    reg64,  OE_RM, "REX.W + 0B /r", }, // r64 OR r/m64.
    { OC_OR,   r64,    mem64,  OE_RM, "REX.W + 0B /r", }, // r64 OR r/m64.
    { OC_XOR,  AL,     imm8,   OE_I,  "34 ib",         }, // AL XOR imm8.
    { OC_XOR,  AX,     imm16,  OE_I,  "35 iw",         }, // AX XOR imm16.
    { OC_XOR,  EAX,    imm32,  OE_I,  "35 id",         }, // EAX XOR imm32.
    { OC_XOR,  RAX,    imm32,  OE_I,  "REX.W + 35 id", }, // RAX XOR imm32 (sign-extended).
    { OC_XOR,  reg8,   imm8,   OE_MI, "80 /6 ib",      }, // r/m8 XOR imm8.
    { OC_XOR,  mem8,   imm8,   OE_MI, "80 /6 ib",      }, // r/m8 XOR imm8.
    { OC_XOR,  reg8*,  imm8,   OE_MI, "REX + 80 /6 ib",}, // r/m8 XOR imm8.
    { OC_XOR,  mem8*,  imm8,   OE_MI, "REX + 80 /6 ib",}, // r/m8 XOR imm8.
    { OC_XOR,  reg16,  imm16,  OE_MI, "81 /6 iw",      }, // r/m16 XOR imm16.
    { OC_XOR,  mem16,  imm16,  OE_MI, "81 /6 iw",      }, // r/m16 XOR imm16.
    { OC_XOR,  reg32,  imm32,  OE_MI, "81 /6 id",      }, // r/m32 XOR imm32.
    { OC_XOR,  mem32,  imm32,  OE_MI, "81 /6 id",      }, // r/m32 XOR imm32.
    { OC_XOR,  reg64,  imm32,  OE_MI, "REX.W + 81 /6 id",  }, // r/m64 XOR imm32 (sign-extended).
    { OC_XOR,  mem64,  imm32,  OE_MI, "REX.W + 81 /6 id",  }, // r/m64 XOR imm32 (sign-extended).
    { OC_XOR,  reg16,  imm8,   OE_MI, "83 /6 ib",      }, // r/m16 XOR imm8 (sign-extended).
    { OC_XOR,  mem16,  imm8,   OE_MI, "83 /6 ib",      }, // r/m16 XOR imm8 (sign-extended).
    { OC_XOR,  reg32,  imm8,   OE_MI, "83 /6 ib",      }, // r/m32 XOR imm8 (sign-extended).
    { OC_XOR,  mem32,  imm8,   OE_MI, "83 /6 ib",      }, // r/m32 XOR imm8 (sign-extended).
    { OC_XOR,  reg64,  imm8,   OE_MI, "RREX.W + 83 /6 ib",   }, // r/m64 XOR imm8 (sign-extended).
    { OC_XOR,  mem64,  imm8,   OE_MI, "RREX.W + 83 /6 ib",   }, // r/m64 XOR imm8 (sign-extended).
    { OC_XOR,  reg8,   r8,     OE_MR, "30 /r",         }, // r/m8 XOR r8.
    { OC_XOR,  mem8,   r8,     OE_MR, "30 /r",         }, // r/m8 XOR r8.
    { OC_XOR,  reg8*,  r8*,    OE_MR, "REX + 30 /r",   }, // r/m8 XOR r8.
    { OC_XOR,  mem8*,  r8*,    OE_MR, "REX + 30 /r",   }, // r/m8 XOR r8.
    { OC_XOR,  reg16,  r16,    OE_MR, "31 /r",         }, // r/m16 XOR r16.
    { OC_XOR,  mem16,  r16,    OE_MR, "31 /r",         }, // r/m16 XOR r16.
    { OC_XOR,  reg32,  r32,    OE_MR, "31 /r",         }, // r/m32 XOR r32.
    { OC_XOR,  mem32,  r32,    OE_MR, "31 /r",         }, // r/m32 XOR r32.
    { OC_XOR,  reg64,  r64,    OE_MR, "REX.W + 31 /r", }, // r/m64 XOR r64.
    { OC_XOR,  mem64,  r64,    OE_MR, "REX.W + 31 /r", }, // r/m64 XOR r64.
    { OC_XOR,  r8,     reg8,   OE_RM, "32 /r",         }, // r8 XOR r/m8.
    { OC_XOR,  r8,     mem8,   OE_RM, "32 /r",         }, // r8 XOR r/m8.
    { OC_XOR,  r8*,    reg8*,  OE_RM, "REX + 32 /r",   }, // r8 XOR r/m8.
    { OC_XOR,  r8*,    mem8*,  OE_RM, "REX + 32 /r",   }, // r8 XOR r/m8.
    { OC_XOR,  r16,    reg16,  OE_RM, "33 /r",         }, // r16 XOR r/m16.
    { OC_XOR,  r16,    mem16,  OE_RM, "33 /r",         }, // r16 XOR r/m16.
    { OC_XOR,  r32,    reg32,  OE_RM, "33 /r",         }, // r32 XOR r/m32.
    { OC_XOR,  r32,    mem32,  OE_RM, "33 /r",         }, // r32 XOR r/m32.
    { OC_XOR,  r64,    reg64,  OE_RM, "REX.W + 33 /r", }, // r64 XOR r/m64.
    { OC_XOR,  r64,    mem64,  OE_RM, "REX.W + 33 /r", }, // r64 XOR r/m64.
    { OC_NOT,  reg8,   0,      OE_M,  "F6 /2",         }, // Reverse each bit of r/m8.
    { OC_NOT,  mem8,   0,      OE_M,  "F6 /2",         }, // Reverse each bit of r/m8.
    { OC_NOT,  reg81,  0,      OE_M,  "REX + F6 /2",   }, // Reverse each bit of r/m8.
    { OC_NOT,  mem81,  0,      OE_M,  "REX + F6 /2",   }, // Reverse each bit of r/m8.
    { OC_NOT,  reg16,  0,      OE_M,  "F7 /2",         }, // Reverse each bit of r/m16.
    { OC_NOT,  mem16,  0,      OE_M,  "F7 /2",         }, // Reverse each bit of r/m16.
    { OC_NOT,  reg32,  0,      OE_M,  "F7 /2",         }, // Reverse each bit of r/m32.
    { OC_NOT,  mem32,  0,      OE_M,  "F7 /2",         }, // Reverse each bit of r/m32.
    { OC_NOT,  reg64,  0,      OE_M,  "REX.W + F7 /2", }, // Reverse each bit of r/m64.
    { OC_NOT,  mem64,  0,      OE_M,  "REX.W + F7 /2", }, // Reverse each bit of r/m64.
    { OC_NEG,  reg8,   0,      OE_M,  "F6 /3",         }, // Two's complement negate r/m8.
    { OC_NEG,  mem8,   0,      OE_M,  "F6 /3",         }, // Two's complement negate r/m8.
    { OC_NEG,  reg81,  0,      OE_M,  "REX + F6 /3",   }, // Two's complement negate r/m8.
    { OC_NEG,  mem81,  0,      OE_M,  "REX + F6 /3",   }, // Two's complement negate r/m8.
    { OC_NEG,  reg16,  0,      OE_M,  "F7 /3",         }, // Two's complement negate r/m16.
    { OC_NEG,  mem16,  0,      OE_M,  "F7 /3",         }, // Two's complement negate r/m16.
    { OC_NEG,  reg32,  0,      OE_M,  "F7 /3",         }, // Two's complement negate r/m32.
    { OC_NEG,  mem32,  0,      OE_M,  "F7 /3",         }, // Two's complement negate r/m32.
    { OC_NEG,  reg64,  0,      OE_M,  "REX.W + F7 /3", }, // Two's complement negate r/m64.
    { OC_NEG,  mem64,  0,      OE_M,  "REX.W + F7 /3", }, // Two's complement negate r/m64.
    { OC_SHL,  reg8,   1,      OE_M1, "D0 /4",         }, // Multiply r/m8 by 2, once.
    { OC_SHL,  mem8,   1,      OE_M1, "D0 /4",         }, // Multiply r/m8 by 2, once.
    { OC_SHL,  reg82,  1,      OE_M1, "REX + D0 /4",   }, // Multiply r/m8 by 2, once.
    { OC_SHL,  mem82,  1,      OE_M1, "REX + D0 /4",   }, // Multiply r/m8 by 2, once.
    { OC_SHL,  reg8,   CL,     OE_MC, "D2 /4",         }, // Multiply r/m8 by 2, CL times.
    { OC_SHL,  mem8,   CL,     OE_MC, "D2 /4",         }, // Multiply r/m8 by 2, CL times.
    { OC_SHL,  reg82,  CL,     OE_MC, "REX + D2 /4",   }, // Multiply r/m8 by 2, CL times.
    { OC_SHL,  mem82,  CL,     OE_MC, "REX + D2 /4",   }, // Multiply r/m8 by 2, CL times.
    { OC_SHL,  reg8,   imm8,   OE_MI, "C0 /4 ib",      }, // Multiply r/m8 by 2, imm8 times.
    { OC_SHL,  mem8,   imm8,   OE_MI, "C0 /4 ib",      }, // Multiply r/m8 by 2, imm8 times.
    { OC_SHL,  reg82,  imm8,   OE_MI, "REX + C0 /4 ib",}, // Multiply r/m8 by 2, imm8 times.
    { OC_SHL,  mem82,  imm8,   OE_MI, "REX + C0 /4 ib",}, // Multiply r/m8 by 2, imm8 times.
    { OC_SHL,  reg16,1,  0,      OE_M1, "D1 /4",         }, // Multiply r/m16 by 2, once.
    { OC_SHL,  mem16,1,  0,      OE_M1, "D1 /4",         }, // Multiply r/m16 by 2, once.
    { OC_SHL,  reg16,  CL,     OE_MC, "D3 /4",         }, // Multiply r/m16 by 2, CL times.
    { OC_SHL,  mem16,  CL,     OE_MC, "D3 /4",         }, // Multiply r/m16 by 2, CL times.
    { OC_SHL,  reg16,  imm8,   OE_MI, "C1 /4 ib",      }, // Multiply r/m16 by 2, imm8 times.
    { OC_SHL,  mem16,  imm8,   OE_MI, "C1 /4 ib",      }, // Multiply r/m16 by 2, imm8 times.
    { OC_SHL,  reg32,1,  0,      OE_M1, "D1 /4",         }, // Multiply r/m32 by 2, once.
    { OC_SHL,  mem32,1,  0,      OE_M1, "D1 /4",         }, // Multiply r/m32 by 2, once.
    { OC_SHL,  reg64,1,  0,      OE_M1, "REX.W + D1 /4", }, // Multiply r/m64 by 2, once.
    { OC_SHL,  mem64,1,  0,      OE_M1, "REX.W + D1 /4", }, // Multiply r/m64 by 2, once.
    { OC_SHL,  reg32,  CL,     OE_MC, "D3 /4",         }, // Multiply r/m32 by 2, CL times.
    { OC_SHL,  mem32,  CL,     OE_MC, "D3 /4",         }, // Multiply r/m32 by 2, CL times.
    { OC_SHL,  reg64,  CL,     OE_MC, "REX.W + D3 /4", }, // Multiply r/m64 by 2, CL times.
    { OC_SHL,  mem64,  CL,     OE_MC, "REX.W + D3 /4", }, // Multiply r/m64 by 2, CL times.
    { OC_SHL,  reg32,  imm8,   OE_MI, "C1 /4 ib",      }, // Multiply r/m32 by 2, imm8 times.
    { OC_SHL,  mem32,  imm8,   OE_MI, "C1 /4 ib",      }, // Multiply r/m32 by 2, imm8 times.
    { OC_SHL,  reg64,  imm8,   OE_MI, "REX.W + C1 /4 ib",  }, // Multiply r/m64 by 2, imm8 times.
    { OC_SHL,  mem64,  imm8,   OE_MI, "REX.W + C1 /4 ib",  }, // Multiply r/m64 by 2, imm8 times.
    { OC_SHR,  reg8,1, 0,      OE_M1, "D0 /5",         }, // Unsigned divide r/m8 by 2, once.
    { OC_SHR,  mem8,1, 0,      OE_M1, "D0 /5",         }, // Unsigned divide r/m8 by 2, once.
    { OC_SHR,  reg82,  1,      OE_M1, "REX + D0 /5",   }, // Unsigned divide r/m8 by 2, once.
    { OC_SHR,  mem82,  1,      OE_M1, "REX + D0 /5",   }, // Unsigned divide r/m8 by 2, once.
    { OC_SHR,  reg8,   CL,     OE_MC, "D2 /5",         }, // Unsigned divide r/m8 by 2, CL times.
    { OC_SHR,  mem8,   CL,     OE_MC, "D2 /5",         }, // Unsigned divide r/m8 by 2, CL times.
    { OC_SHR,  reg82,  CL,     OE_MC, "REX + D2 /5",   }, // Unsigned divide r/m8 by 2, CL times.
    { OC_SHR,  mem82,  CL,     OE_MC, "REX + D2 /5",   }, // Unsigned divide r/m8 by 2, CL times.
    { OC_SHR,  reg8,   imm8,   OE_MI, "C0 /5 ib",      }, // Unsigned divide r/m8 by 2, imm8 times.
    { OC_SHR,  mem8,   imm8,   OE_MI, "C0 /5 ib",      }, // Unsigned divide r/m8 by 2, imm8 times.
    { OC_SHR,  reg82,  imm8,   OE_MI, "REX + C0 /5 ib",}, // Unsigned divide r/m8 by 2, imm8 times.
    { OC_SHR,  mem82,  imm8,   OE_MI, "REX + C0 /5 ib",}, // Unsigned divide r/m8 by 2, imm8 times.
    { OC_SHR,  reg16,  1,      OE_M1, "D1 /5",         }, // Unsigned divide r/m16 by 2, once.
    { OC_SHR,  mem16,  1,      OE_M1, "D1 /5",         }, // Unsigned divide r/m16 by 2, once.
    { OC_SHR,  reg16,  CL,     OE_MC, "D3 /5",         }, // Unsigned divide r/m16 by 2, CL times
    { OC_SHR,  mem16,  CL,     OE_MC, "D3 /5",         }, // Unsigned divide r/m16 by 2, CL times
    { OC_SHR,  reg16,  imm8,   OE_MI, "C1 /5 ib",      }, // Unsigned divide r/m16 by 2, imm8 times.
    { OC_SHR,  mem16,  imm8,   OE_MI, "C1 /5 ib",      }, // Unsigned divide r/m16 by 2, imm8 times.
    { OC_SHR,  reg32,  1,      OE_M1, "D1 /5",         }, // Unsigned divide r/m32 by 2, once.
    { OC_SHR,  mem32,  1,      OE_M1, "D1 /5",         }, // Unsigned divide r/m32 by 2, once.
    { OC_SHR,  reg64,  1,      OE_M1, "REX.W + D1 /5", }, // Unsigned divide r/m64 by 2, once.
    { OC_SHR,  mem64,  1,      OE_M1, "REX.W + D1 /5", }, // Unsigned divide r/m64 by 2, once.
    { OC_SHR,  reg32,  CL,     OE_MC, "D3 /5",         }, // Unsigned divide r/m32 by 2, CL times.
    { OC_SHR,  mem32,  CL,     OE_MC, "D3 /5",         }, // Unsigned divide r/m32 by 2, CL times.
    { OC_SHR,  reg64,  CL,     OE_MC, "REX.W + D3 /5", }, // Unsigned divide r/m64 by 2, CL times.
    { OC_SHR,  mem64,  CL,     OE_MC, "REX.W + D3 /5", }, // Unsigned divide r/m64 by 2, CL times.
    { OC_SHR,  reg32,  imm8,   OE_MI, "C1 /5 ib",      }, // Unsigned divide r/m32 by 2, imm8 times.
    { OC_SHR,  mem32,  imm8,   OE_MI, "C1 /5 ib",      }, // Unsigned divide r/m32 by 2, imm8 times.
    { OC_SHR,  reg64,  imm8,   OE_MI, "REX.W + C1 /5 ib",  }, // Unsigned divide r/m64 by 2, imm8 times.
    { OC_SHR,  mem64,  imm8,   OE_MI, "REX.W + C1 /5 ib",  }, // Unsigned divide r/m64 by 2, imm8 times.
    { OC_JMP,  rel8,   0,      OE_D,  "EB cb",         }, // Jump short, RIP = RIP + 8-bit displacement sign extended to 64-bits.
    { OC_JMP,  rel32,  0,      OE_D,  "E9 cd",         }, // Jump near, relative, RIP = RIP + 32-bit displacement sign extended to 64-bits.
    { OC_JMP,  reg64,  0,      OE_M,  "FF /4",         }, // Jump near, absolute indirect, RIP = 64-Bit offset from register or memory.
    { OC_JMP,  mem64,  0,      OE_M,  "FF /4",         }, // Jump near, absolute indirect, RIP = 64-Bit offset from register or memory.
    { OC_JMP,  m16:16, 0,      OE_M,  "FF /5",         }, // Jump far, absolute indirect, address given in m16:16.
    { OC_JMP,  m16:32, 0,      OE_M,  "FF /5",         }, // Jump far, absolute indirect, address given in m16:32.
    { OC_JMP,  m16:64, 0,      OE_M,  "REX.W FF /5",   }, // Jump far, absolute indirect, address given in m16:64.
    { OC_CMP,  AL,     imm8,   OE_I,  "3C ib",         }, // Compare imm8 with AL.
    { OC_CMP,  AX,     imm16,  OE_I,  "3D iw",         }, // Compare imm16 with AX.
    { OC_CMP,  EAX,    imm32,  OE_I,  "3D id",         }, // Compare imm32 with EAX.
    { OC_CMP,  RAX,    imm32,  OE_I,  "REX.W + 3D id", }, // Compare imm32 sign-extended to 64-bits with RAX.
    { OC_CMP,  reg8,   imm8,   OE_MI, "80 /7 ib",      }, // Compare imm8 with r/m8.
    { OC_CMP,  mem8,   imm8,   OE_MI, "80 /7 ib",      }, // Compare imm8 with r/m8.
    { OC_CMP,  reg8*,  imm8,   OE_MI, "REX + 80 /7 ib",}, // Compare imm8 with r/m8.
    { OC_CMP,  mem8*,  imm8,   OE_MI, "REX + 80 /7 ib",}, // Compare imm8 with r/m8.
    { OC_CMP,  reg16,  imm16,  OE_MI, "81 /7 iw",      }, // Compare imm16 with r/m16.
    { OC_CMP,  mem16,  imm16,  OE_MI, "81 /7 iw",      }, // Compare imm16 with r/m16.
    { OC_CMP,  reg32,  imm32,  OE_MI, "81 /7 id",      }, // Compare imm32 with r/m32.
    { OC_CMP,  mem32,  imm32,  OE_MI, "81 /7 id",      }, // Compare imm32 with r/m32.
    { OC_CMP,  reg64,  imm32,  OE_MI, "REX.W + 81 /7 id",  }, // Compare imm32 sign-extended to 64-bits with r/m64.
    { OC_CMP,  mem64,  imm32,  OE_MI, "REX.W + 81 /7 id",  }, // Compare imm32 sign-extended to 64-bits with r/m64.
    { OC_CMP,  reg16,  imm8,   OE_MI, "83 /7 ib",      }, // Compare imm8 with r/m16.
    { OC_CMP,  mem16,  imm8,   OE_MI, "83 /7 ib",      }, // Compare imm8 with r/m16.
    { OC_CMP,  reg32,  imm8,   OE_MI, "83 /7 ib",      }, // Compare imm8 with r/m32.
    { OC_CMP,  mem32,  imm8,   OE_MI, "83 /7 ib",      }, // Compare imm8 with r/m32.
    { OC_CMP,  reg64,  imm8,   OE_MI, "REX.W + 83 /7 ib",  }, // Compare imm8 with r/m64.
    { OC_CMP,  mem64,  imm8,   OE_MI, "REX.W + 83 /7 ib",  }, // Compare imm8 with r/m64.
    { OC_CMP,  reg8,   r8,     OE_MR, "38 /r",         }, // Compare r8 with r/m8.
    { OC_CMP,  mem8,   r8,     OE_MR, "38 /r",         }, // Compare r8 with r/m8.
    { OC_CMP,  reg8*,  r8*,    OE_MR, "REX + 38 /r",   }, // Compare r8 with r/m8.
    { OC_CMP,  mem8*,  r8*,    OE_MR, "REX + 38 /r",   }, // Compare r8 with r/m8.
    { OC_CMP,  reg16,  r16,    OE_MR, "39 /r",         }, // Compare r16 with r/m16.
    { OC_CMP,  mem16,  r16,    OE_MR, "39 /r",         }, // Compare r16 with r/m16.
    { OC_CMP,  reg32,  r32,    OE_MR, "39 /r",         }, // Compare r32 with r/m32.
    { OC_CMP,  mem32,  r32,    OE_MR, "39 /r",         }, // Compare r32 with r/m32.
    { OC_CMP,  reg64,r64,    0,      OE_MR, "REX.W + 39 /r", }, // Compare r64 with r/m64.
    { OC_CMP,  mem64,r64,    0,      OE_MR, "REX.W + 39 /r", }, // Compare r64 with r/m64.
    { OC_CMP,  r8,     reg8,   OE_RM, "3A /r",         }, // Compare r/m8 with r8.
    { OC_CMP,  r8,     mem8,   OE_RM, "3A /r",         }, // Compare r/m8 with r8.
    { OC_CMP,  r8*,    reg8*,  OE_RM, "REX + 3A /r",   }, // Compare r/m8 with r8.
    { OC_CMP,  r8*,    mem8*,  OE_RM, "REX + 3A /r",   }, // Compare r/m8 with r8.
    { OC_CMP,  r16,    reg16,  OE_RM, "3B /r",         }, // Compare r/m16 with r16.
    { OC_CMP,  r16,    mem16,  OE_RM, "3B /r",         }, // Compare r/m16 with r16.
    { OC_CMP,  r32,    reg32,  OE_RM, "3B /r",         }, // Compare r/m32 with r32.
    { OC_CMP,  r32,    mem32,  OE_RM, "3B /r",         }, // Compare r/m32 with r32.
    { OC_CMP,  r64,    reg64,  OE_RM, "REX.W + 3B /r", }, // Compare r/m64 with r64.
    { OC_CMP,  r64,    mem64,  OE_RM, "REX.W + 3B /r", }, // Compare r/m64 with r64.
    { OC_JA,   rel8,   0,      OE_D,  "77 cb",         }, // Jump short if above (CF=0 and ZF=0).
    { OC_JA,   rel32,  0,      OE_D,  "0F 87 cd",      }, // Jump near if above (CF=0 and ZF=0).
    { OC_JAE,  rel8,   0,      OE_D,  "73 cb",         }, // Jump short if above or equal (CF=0).
    { OC_JAE,  rel32,  0,      OE_D,  "0F 83 cd",      }, // Jump near if above or equal (CF=0).
    { OC_JB,   rel8,   0,      OE_D,  "72 cb",         }, // Jump short if below (CF=1).
    { OC_JB,   rel32,  0,      OE_D,  "0F 82 cd",      }, // Jump near if below (CF=1).
    { OC_JBE,  rel8,   0,      OE_D,  "76 cb",         }, // Jump short if below or equal (CF=1 or ZF=1).
    { OC_JBE,  rel32,  0,      OE_D,  "0F 86 cd",      }, // Jump near if below or equal (CF=1 or ZF=1).
    { OC_JC,   rel8,   0,      OE_D,  "72 cb",         }, // Jump short if carry (CF=1).
    { OC_JC,   rel32,  0,      OE_D,  "0F 82 cd",      }, // Jump near if carry (CF=1).
    { OC_JECXZ,  rel8,   0,      OE_D,  "E3 cb",         }, // Jump short if ECX register is 0.
    { OC_JRCXZ,  rel8,   0,      OE_D,  "E3 cb",         }, // Jump short if RCX register is 0.
    { OC_JE,   rel8,   0,      OE_D,  "74 cb",         }, // Jump short if equal (ZF=1).
    { OC_JE,   rel32,  0,      OE_D,  "0F 84 cd",      }, // Jump near if equal (ZF=1).
    { OC_JZ,   rel32,  0,      OE_D,  "0F 84 cd",      }, // Jump near if 0 (ZF=1).
    { OC_JG,   rel8,   0,      OE_D,  "7F cb",         }, // Jump short if greater (ZF=0 and SF=OF).
    { OC_JG,   rel32,  0,      OE_D,  "0F 8F cd",      }, // Jump near if greater (ZF=0 and SF=OF).
    { OC_JGE,  rel8,   0,      OE_D,  "7D cb",         }, // Jump short if greater or equal (SF=OF).
    { OC_JGE,  rel32,  0,      OE_D,  "0F 8D cd",      }, // Jump near if greater or equal (SF=OF).
    { OC_JL,   rel8,   0,      OE_D,  "7C cb",         }, // Jump short if less (SF≠ OF).
    { OC_JL,   rel32,  0,      OE_D,  "0F 8C cd",      }, // Jump near if less (SF≠ OF).
    { OC_JLE,  rel8,   0,      OE_D,  "7E cb",         }, // Jump short if less or equal (ZF=1 or SF≠ OF).
    { OC_JLE,  rel32,  0,      OE_D,  "0F 8E cd",      }, // Jump near if less or equal (ZF=1 or SF≠ OF).
    { OC_JNA,  rel8,   0,      OE_D,  "76 cb",         }, // Jump short if not above (CF=1 or ZF=1).
    { OC_JNA,  rel32,  0,      OE_D,  "0F 86 cd",      }, // Jump near if not above (CF=1 or ZF=1).
    { OC_JNAE, rel8,   0,      OE_D,  "72 cb",         }, // Jump short if not above or equal (CF=1).
    { OC_JNAE, rel32,  0,      OE_D,  "0F 82 cd",      }, // Jump near if not above or equal (CF=1).
    { OC_JNB,  rel8,   0,      OE_D,  "73 cb",         }, // Jump short if not below (CF=0).
    { OC_JNB,  rel32,  0,      OE_D,  "0F 83 cd",      }, // Jump near if not below (CF=0).
    { OC_JNBE, rel8,   0,      OE_D,  "77 cb",         }, // Jump short if not below or equal (CF=0 and ZF=0).
    { OC_JNBE, rel32,  0,      OE_D,  "0F 87 cd",      }, // Jump near if not below or equal (CF=0 and ZF=0).
    { OC_JNC,  rel8,   0,      OE_D,  "73 cb",         }, // Jump short if not carry (CF=0).
    { OC_JNC,  rel32,  0,      OE_D,  "0F 83 cd",      }, // Jump near if not carry (CF=0).
    { OC_JNE,  rel8,   0,      OE_D,  "75 cb",         }, // Jump short if not equal (ZF=0).
    { OC_JNE,  rel32,  0,      OE_D,  "0F 85 cd",      }, // Jump near if not equal (ZF=0).
    { OC_JNG,  rel8,   0,      OE_D,  "7E cb",         }, // Jump short if not greater (ZF=1 or SF≠ OF).
    { OC_JNG,  rel32,  0,      OE_D,  "0F 8E cd",      }, // Jump near if not greater (ZF=1 or SF≠ OF).
    { OC_JNGE, rel8,   0,      OE_D,  "7C cb",         }, // Jump short if not greater or equal (SF≠ OF).
    { OC_JNGE, rel32,  0,      OE_D,  "0F 8C cd",      }, // Jump near if not greater or equal (SF≠ OF).
    { OC_JNL,  rel8,   0,      OE_D,  "7D cb",         }, // Jump short if not less (SF=OF).
    { OC_JNL,  rel32,  0,      OE_D,  "0F 8D cd",      }, // Jump near if not less (SF=OF).
    { OC_JNLE, rel8,   0,      OE_D,  "7F cb",         }, // Jump short if not less or equal (ZF=0 and SF=OF).
    { OC_JNLE, rel32,  0,      OE_D,  "0F 8F cd",      }, // Jump near if not less or equal (ZF=0 and SF=OF).
    { OC_JNO,  rel8,   0,      OE_D,  "71 cb",         }, // Jump short if not overflow (OF=0).
    { OC_JNO,  rel32,  0,      OE_D,  "0F 81 cd",      }, // Jump near if not overflow (OF=0).
    { OC_JNP,  rel8,   0,      OE_D,  "7B cb",         }, // Jump short if not parity (PF=0).
    { OC_JNP,  rel32,  0,      OE_D,  "0F 8B cd",      }, // Jump near if not parity (PF=0).
    { OC_JNS,  rel8,   0,      OE_D,  "79 cb",         }, // Jump short if not sign (SF=0).
    { OC_JNS,  rel32,  0,      OE_D,  "0F 89 cd",      }, // Jump near if not sign (SF=0).
    { OC_JNZ,  rel8,   0,      OE_D,  "75 cb",         }, // Jump short if not zero (ZF=0).
    { OC_JNZ,  rel32,  0,      OE_D,  "0F 85 cd",      }, // Jump near if not zero (ZF=0).
    { OC_JO,   rel8,   0,      OE_D,  "70 cb",         }, // Jump short if overflow (OF=1).
    { OC_JO,   rel32,  0,      OE_D,  "0F 80 cd",      }, // Jump near if overflow (OF=1).
    { OC_JP,   rel8,   0,      OE_D,  "7A cb",         }, // Jump short if parity (PF=1).
    { OC_JP,   rel32,  0,      OE_D,  "0F 8A cd",      }, // Jump near if parity (PF=1).
    { OC_JPE,  rel8,   0,      OE_D,  "7A cb",         }, // Jump short if parity even (PF=1).
    { OC_JPE,  rel32,  0,      OE_D,  "0F 8A cd",      }, // Jump near if parity even (PF=1).
    { OC_JPO,  rel8,   0,      OE_D,  "7B cb",         }, // Jump short if parity odd (PF=0).
    { OC_JPO,  rel32,  0,      OE_D,  "0F 8B cd",      }, // Jump near if parity odd (PF=0).
    { OC_JS,   rel8,   0,      OE_D,  "78 cb",         }, // Jump short if sign (SF=1).
    { OC_JS,   rel32,  0,      OE_D,  "0F 88 cd",      }, // Jump near if sign (SF=1).
    { OC_JZ,   rel8,   0,      OE_D,  "74 cb",         }, // Jump short if zero (ZF = 1).
    { OC_JZ,   rel32,  0,      OE_D,  "0F 84 cd",      }, // Jump near if 0 (ZF=1).
    { OC_CALL, rel32,  0,      OE_D,  "E8 cd",         }, // Call near, relative, displacement relative to next instruction. 32-bit displacement sign extended to 64-bits in 64-bit mode.
    { OC_CALL, reg64,  0,      OE_M,  "FF /2",         }, // Call near, absolute indirect, address given in r/m64.
    { OC_CALL, mem64,  0,      OE_M,  "FF /2",         }, // Call near, absolute indirect, address given in r/m64.
    { OC_CALL, m16:16, 0,      OE_M,  "FF /3",         }, // Call far, absolute indirect address given in m16:16. In 32-bit mode: if selector points to a gate, then RIP = 32-bit zero extended displacement taken from gate; else RIP = zero extended 16-bit offset from far pointer referenced in the instruction.
    { OC_CALL, m16:32, 0,      OE_M,  "FF /3",         }, // In 64-bit mode: If selector points to a gate, then RIP = 64-bit displacement taken from gate; else RIP = zero extended 32-bit offset from far pointer referenced in the instruction.
    { OC_CALL, m16:64, 0,      OE_M,  "REX.W FF /3",   }, // In 64-bit mode: If selector points to a gate, then RIP = 64-bit displacement taken from gate; else RIP = 64-bit offset from far pointer referenced in the instruction.
    { OC_RET,  0,      0,      OE_ZO, "C3",            }, // Near return to calling procedure.
    { OC_RET,  0,      0,      OE_ZO, "CB",            }, // Far return to calling procedure.
    { OC_RET,  imm16,  0,      OE_I,  "C2 iw",         }, // Near return to calling procedure and pop imm16 bytes from stack.
    { OC_RET,  imm16,  0,      OE_I,  "CA iw",         }, // Far return to calling procedure and pop imm16 bytes from stack.
    { OC_INT,  imm8,   0,      OE_I,  "CD ib",         }, // Generate software interrupt with vector specified by immediate byte.
};

