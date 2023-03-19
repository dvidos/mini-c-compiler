get_next_counter_value:
    PUSH EBP
    MOV EBP, ESP
    MOV EAX, counter
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-4], 0x1
    MOV EAX, EAX
    ADD EAX, EAX
    MOV counter, EAX
    MOV ret_val, counter
    JMP get_next_counter_value_end
get_next_counter_value_end:
    MOV ESP, EBP
    POP EBP
    RET

rect_area:
    PUSH EBP
    MOV EBP, ESP
    ; [EBP +8] argument "width", 4 bytes
    ; [EBP+12] argument "height", 4 bytes
    MOV EAX, width
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-4], height
    MOV EAX, EAX
    IMUL EAX, EAX
    MOV ret_val, EAX
    JMP rect_area_end
rect_area_end:
    MOV ESP, EBP
    POP EBP
    RET

triangle_area:
    PUSH EBP
    MOV EBP, ESP
    ; [EBP +8] argument "width", 4 bytes
    ; [EBP+12] argument "height", 4 bytes
    MOV EAX, width
    MOV EAX, height
    MOV EAX, 0x2
    MOV EAX, EAX
    IDIV EAX, EAX
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-4], EAX
    MOV EAX, EAX
    IMUL EAX, EAX
    MOV ret_val, EAX
    JMP triangle_area_end
triangle_area_end:
    MOV ESP, EBP
    POP EBP
    RET

circle_area:
    PUSH EBP
    MOV EBP, ESP
    ; [EBP +8] argument "radius", 4 bytes
    MOV E(unknown), 0x3
    MOV EDX, radius
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-4], radius
    MOV EAX, EAX
    IMUL EAX, EAX
    SUB ESP, 0x4         ; grab some space for temp register
    MOV E(unknown), EAX
    MOV EAX, EAX
    IMUL EAX, EAX
    MOV ret_val, EAX
    JMP circle_area_end
circle_area_end:
    MOV ESP, EBP
    POP EBP
    RET

fibonacci:
    PUSH EBP
    MOV EBP, ESP
    ; [EBP +8] argument "n", 4 bytes
    CMP n, 0x2
    JGT if_3_end
    MOV ret_val, n
    JMP fibonacci_end
if_3_end:
    MOV EAX, n
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-4], 0x1
    MOV EAX, EAX
    SUB EAX, EAX
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-8], EAX
    PUSH [EBP-8]
    CALL fibonacci
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-12], EAX
    ADD ESP, 0x4
    MOV EAX, n
    SUB ESP, 0x4         ; grab some space for temp register
    MOV EAX, 0x2
    MOV EAX, EAX
    SUB EAX, EAX
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-20], EAX
    PUSH [EBP-20]
    CALL fibonacci
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-24], EAX
    ADD ESP, 0x4
    MOV EAX, [EBP-12]
    ADD EAX, [EBP-12]
    MOV ret_val, EAX
    JMP fibonacci_end
fibonacci_end:
    MOV ESP, EBP
    POP EBP
    RET

factorial:
    PUSH EBP
    MOV EBP, ESP
    ; [EBP +8] argument "n", 4 bytes
    CMP n, 0x1
    JGT if_4_end
    MOV ret_val, n
    JMP factorial_end
if_4_end:
    MOV E(unknown), n
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-4], n
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-8], 0x1
    MOV EAX, [EBP-4]
    SUB EAX, [EBP-4]
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-12], EAX
    PUSH [EBP-12]
    CALL factorial
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-16], EAX
    ADD ESP, 0x4
    MOV EAX, EAX
    IMUL EAX, EAX
    MOV ret_val, EAX
    JMP factorial_end
factorial_end:
    MOV ESP, EBP
    POP EBP
    RET

math_demo:
    PUSH EBP
    MOV EBP, ESP
    SUB ESP, 0x4         ; space for local vars
    ; [EBP -4] local var "i", 4 bytes
    MOV i, 0x0
while_5_begin:
    MOV EAX, i
    MOV EAX, i
    ADD EAX, i
    MOV i, EAX
    CMP EAX, 0xa
    JGE while_5_end
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-8], __str_1
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-12], i
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-16], i
    PUSH [EBP-16]
    CALL fibonacci
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-20], EAX
    ADD ESP, 0x4
    PUSH [EBP-20]
    PUSH [EBP-12]
    PUSH [EBP-8]
    CALL printf
    ADD ESP, 0xc
    JMP while_5_begin
while_5_end:
    MOV i, 0x0
while_6_begin:
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-24], i
    MOV EAX, i
    ADD EAX, i
    MOV i, EAX
    CMP [EBP-24], 0xa
    JGE while_6_end
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-28], __str_2
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-32], i
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-36], i
    PUSH [EBP-36]
    CALL factorial
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-40], EAX
    ADD ESP, 0x4
    PUSH [EBP-40]
    PUSH [EBP-32]
    PUSH [EBP-28]
    CALL printf
    ADD ESP, 0xc
    JMP while_6_begin
math_demo_end:
    MOV ESP, EBP
    POP EBP
    RET

nested_loops_test:
    PUSH EBP
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
nested_loops_test_end:
    MOV ESP, EBP
    POP EBP
    RET

main:
    PUSH EBP
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
    MOV EAX, a
    MOV E(unknown), 0x2
    MOV EAX, EAX
    ADD EAX, EAX
    MOV b, EAX
    CALL math_demo
    ADD ESP, 0x0
main_end:
    MOV ESP, EBP
    POP EBP
    RET

