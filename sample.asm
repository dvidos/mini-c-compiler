get_next_counter_value:
    PUSH EBP             ; establish stak frame
    MOV EBP, ESP
    MOV EBX, counter
    MOV ECX, 0x1
    MOV EAX, EBX
    ADD EAX, EBX
    MOV counter, EAX
    MOV ECX, counter
    MOV EAX, ECX         ; put returned value in AX
    JMP get_next_counter_value_exit
get_next_counter_value_exit:
    MOV ESP, EBP         ; tear down stak frame
    POP EBP
    RET

rect_area:
    PUSH EBP             ; establish stak frame
    MOV EBP, ESP
    ; [EBP +8] argument "width", 4 bytes
    ; [EBP+12] argument "height", 4 bytes
    MOV EBX, width
    MOV ECX, height
    MOV EAX, EBX
    IMUL EAX, EBX
    MOV EDX, EAX
    MOV EAX, EDX         ; put returned value in AX
    JMP rect_area_exit
rect_area_exit:
    MOV ESP, EBP         ; tear down stak frame
    POP EBP
    RET

triangle_area:
    PUSH EBP             ; establish stak frame
    MOV EBP, ESP
    ; [EBP +8] argument "width", 4 bytes
    ; [EBP+12] argument "height", 4 bytes
    MOV EBX, width
    MOV ECX, height
    MOV EDX, 0x2
    MOV EAX, ECX
    IDIV EAX, ECX
    MOV E(unknown), EAX
    MOV EAX, EBX
    IMUL EAX, EBX
    MOV E(unknown), EAX
    MOV EAX, E(unknown)  ; put returned value in AX
    JMP triangle_area_exit
triangle_area_exit:
    MOV ESP, EBP         ; tear down stak frame
    POP EBP
    RET

circle_area:
    PUSH EBP             ; establish stak frame
    MOV EBP, ESP
    ; [EBP +8] argument "radius", 4 bytes
    MOV EBX, 0x3
    MOV ECX, radius
    MOV EDX, radius
    MOV EAX, ECX
    IMUL EAX, ECX
    MOV E(unknown), EAX
    MOV EAX, EBX
    IMUL EAX, EBX
    MOV E(unknown), EAX
    MOV EAX, E(unknown)  ; put returned value in AX
    JMP circle_area_exit
circle_area_exit:
    MOV ESP, EBP         ; tear down stak frame
    POP EBP
    RET

fibonacci:
    PUSH EBP             ; establish stak frame
    MOV EBP, ESP
    ; [EBP +8] argument "n", 4 bytes
    CMP n, 0x2
    JGT if_3_end
    MOV EBX, n
    MOV EAX, EBX         ; put returned value in AX
    JMP fibonacci_exit
if_3_end:
    MOV ECX, n
    MOV EDX, 0x1
    MOV EAX, ECX
    SUB EAX, ECX
    MOV E(unknown), EAX
    PUSH E(unknown)      ; push 1 args for function call
    CALL fibonacci
    MOV E(unknown), EAX
    ADD ESP, 0x4
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-4], n
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-8], 0x2
    MOV EAX, [EBP-4]
    SUB EAX, [EBP-4]
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-12], EAX
    PUSH [EBP-12]        ; push 1 args for function call
    CALL fibonacci
    SUB ESP, 0x4         ; grab some space for temp register
    MOV E(unknown), EAX
    ADD ESP, 0x4
    MOV EAX, E(unknown)
    ADD EAX, E(unknown)
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-20], EAX
    MOV EAX, [EBP-20]    ; put returned value in AX
    JMP fibonacci_exit
fibonacci_exit:
    MOV ESP, EBP         ; tear down stak frame
    POP EBP
    RET

factorial:
    PUSH EBP             ; establish stak frame
    MOV EBP, ESP
    ; [EBP +8] argument "n", 4 bytes
    CMP n, 0x1
    JGT if_4_end
    MOV EBX, n
    MOV EAX, EBX         ; put returned value in AX
    JMP factorial_exit
if_4_end:
    MOV ECX, n
    MOV EDX, n
    MOV E(unknown), 0x1
    MOV EAX, EDX
    SUB EAX, EDX
    MOV E(unknown), EAX
    PUSH E(unknown)      ; push 1 args for function call
    CALL factorial
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-4], EAX
    ADD ESP, 0x4
    MOV EAX, ECX
    IMUL EAX, ECX
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-8], EAX
    MOV EAX, [EBP-8]     ; put returned value in AX
    JMP factorial_exit
factorial_exit:
    MOV ESP, EBP         ; tear down stak frame
    POP EBP
    RET

math_demo:
    PUSH EBP             ; establish stak frame
    MOV EBP, ESP
    SUB ESP, 0x4         ; space for local vars
    ; [EBP -4] local var "i", 4 bytes
    MOV i, 0x0
while_5_begin:
    MOV EBX, i
    MOV EAX, i
    ADD EAX, i
    MOV i, EAX
    CMP EBX, 0xa
    JGE while_5_end
    MOV ECX, __str_1
    MOV EDX, i
    MOV E(unknown), i
    PUSH E(unknown)      ; push 1 args for function call
    CALL fibonacci
    MOV E(unknown), EAX
    ADD ESP, 0x4
    PUSH E(unknown)      ; push 3 args for function call
    PUSH EDX
    PUSH ECX
    CALL printf
    ADD ESP, 0xc
    JMP while_5_begin
while_5_end:
    MOV i, 0x0
while_6_begin:
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-8], i
    MOV EAX, i
    ADD EAX, i
    MOV i, EAX
    CMP [EBP-8], 0xa
    JGE while_6_end
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-12], __str_2
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-16], i
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-20], i
    PUSH [EBP-20]        ; push 1 args for function call
    CALL factorial
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-24], EAX
    ADD ESP, 0x4
    PUSH [EBP-24]        ; push 3 args for function call
    PUSH [EBP-16]
    PUSH [EBP-12]
    CALL printf
    ADD ESP, 0xc
    JMP while_6_begin
math_demo_exit:
    MOV ESP, EBP         ; tear down stak frame
    POP EBP
    RET

nested_loops_test:
    PUSH EBP             ; establish stak frame
    MOV EBP, ESP
    SUB ESP, 0x8         ; space for local vars
    ; [EBP -4] local var "outer", 4 bytes
    ; [EBP -8] local var "inner", 4 bytes
    MOV outer, 0xa
while_7_begin:
    CMP outer, 0x0
    JLE while_7_end
    MOV inner, 0xf
while_8_begin:
    CMP inner, 0x0
    JLE while_8_end
    MOV EAX, inner
    SUB EAX, inner
    MOV inner, EAX
    JMP while_8_begin
while_8_end:
    MOV EAX, outer
    SUB EAX, outer
    MOV outer, EAX
    JMP while_7_begin
nested_loops_test_exit:
    MOV ESP, EBP         ; tear down stak frame
    POP EBP
    RET

main:
    PUSH EBP             ; establish stak frame
    MOV EBP, ESP
    SUB ESP, 0x1d        ; space for local vars
    ; [EBP +8] argument "argc", 4 bytes
    ; [EBP+12] argument "argv", 4 bytes
    ; [EBP -4] local var "a", 4 bytes
    ; [EBP -8] local var "b", 4 bytes
    ; [EBP -9] local var "c", 1 bytes
    ; [EBP-13] local var "d", 4 bytes
    ; [EBP-29] local var "buffer", 16 bytes
    MOV a, 0x1
    MOV EBX, a
    MOV ECX, 0x2
    MOV EAX, EBX
    ADD EAX, EBX
    MOV b, EAX
    CALL math_demo
    ADD ESP, 0x0
main_exit:
    MOV ESP, EBP         ; tear down stak frame
    POP EBP
    RET

