get_next_counter_value:
    PUSH EBP             ; establish stack frame
    MOV EBP, ESP
    MOV EBX, counter
    MOV ECX, 0x1
    MOV EAX, EBX
    ADD EAX, ECX
    MOV counter, EAX
    MOV EDX, counter
    MOV EAX, EDX         ; put returned value in AX
    JMP get_next_counter_value_exit
get_next_counter_value_exit:
    MOV ESP, EBP         ; tear down stack frame
    POP EBP
    RET

rect_area:
    PUSH EBP             ; establish stack frame
    MOV EBP, ESP
    ; [EBP +8] argument "width", 4 bytes
    ; [EBP+12] argument "height", 4 bytes
    MOV EBX, [EBP+8]
    MOV ECX, [EBP+12]
    MOV EAX, EBX
    IMUL EAX, ECX
    MOV EDX, EAX
    MOV EAX, EDX         ; put returned value in AX
    JMP rect_area_exit
rect_area_exit:
    MOV ESP, EBP         ; tear down stack frame
    POP EBP
    RET

triangle_area:
    PUSH EBP             ; establish stack frame
    MOV EBP, ESP
    ; [EBP +8] argument "width", 4 bytes
    ; [EBP+12] argument "height", 4 bytes
    MOV EBX, [EBP+8]
    MOV ECX, [EBP+12]
    MOV EDX, 0x2
    MOV EAX, ECX
    IDIV EAX, EDX
    MOV ESI, EAX
    MOV EAX, EBX
    IMUL EAX, ESI
    MOV EDI, EAX
    MOV EAX, EDI         ; put returned value in AX
    JMP triangle_area_exit
triangle_area_exit:
    MOV ESP, EBP         ; tear down stack frame
    POP EBP
    RET

circle_area:
    PUSH EBP             ; establish stack frame
    MOV EBP, ESP
    ; [EBP +8] argument "radius", 4 bytes
    MOV EBX, 0x3
    MOV ECX, [EBP+8]
    MOV EDX, [EBP+8]
    MOV EAX, ECX
    IMUL EAX, EDX
    MOV ESI, EAX
    MOV EAX, EBX
    IMUL EAX, ESI
    MOV EDI, EAX
    MOV EAX, EDI         ; put returned value in AX
    JMP circle_area_exit
circle_area_exit:
    MOV ESP, EBP         ; tear down stack frame
    POP EBP
    RET

fibonacci:
    PUSH EBP             ; establish stack frame
    MOV EBP, ESP
    ; [EBP +8] argument "n", 4 bytes
    CMP [EBP+8], 0x2
    JGT if_3_end
    MOV EBX, [EBP+8]
    MOV EAX, EBX         ; put returned value in AX
    JMP fibonacci_exit
if_3_end:
    MOV ECX, [EBP+8]
    MOV EDX, 0x1
    MOV EAX, ECX
    SUB EAX, EDX
    MOV ESI, EAX
    PUSH ESI             ; push 1 args for function call
    CALL fibonacci
    MOV EDI, EAX         ; get value returned from function
    ADD ESP, 0x4         ; clean up 4 bytes that were pushed
    SUB ESP, 0x4         ; grab some space for temp register
    MOV E(unknown), [EBP+8]
    SUB ESP, 0x4         ; grab some space for temp register
    MOV EAX, 0x2
    MOV EAX, E(unknown)
    SUB EAX, EAX
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-12], EAX
    PUSH [EBP-12]        ; push 1 args for function call
    CALL fibonacci
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-16], EAX
    ADD ESP, 0x4         ; clean up 4 bytes that were pushed
    MOV EAX, EDI
    ADD EAX, [EBP-16]
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-20], EAX
    MOV EAX, [EBP-20]    ; put returned value in AX
    JMP fibonacci_exit
fibonacci_exit:
    MOV ESP, EBP         ; tear down stack frame
    POP EBP
    RET

factorial:
    PUSH EBP             ; establish stack frame
    MOV EBP, ESP
    ; [EBP +8] argument "n", 4 bytes
    CMP [EBP+8], 0x1
    JGT if_4_end
    MOV EBX, [EBP+8]
    MOV EAX, EBX         ; put returned value in AX
    JMP factorial_exit
if_4_end:
    MOV ECX, [EBP+8]
    MOV EDX, [EBP+8]
    MOV ESI, 0x1
    MOV EAX, EDX
    SUB EAX, ESI
    MOV EDI, EAX
    PUSH EDI             ; push 1 args for function call
    CALL factorial
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-4], EAX
    ADD ESP, 0x4         ; clean up 4 bytes that were pushed
    MOV EAX, ECX
    IMUL EAX, [EBP-4]
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-8], EAX
    MOV EAX, [EBP-8]     ; put returned value in AX
    JMP factorial_exit
factorial_exit:
    MOV ESP, EBP         ; tear down stack frame
    POP EBP
    RET

math_demo:
    PUSH EBP             ; establish stack frame
    MOV EBP, ESP
    SUB ESP, 0x4         ; space for local vars
    ; [EBP -4] local var "i", 4 bytes
    MOV [EBP-4], 0x0
while_5_begin:
    MOV EBX, [EBP-4]
    MOV EAX, EBX
    ADD EAX, 0x1
    MOV [EBP-4], EAX
    MOV ECX, EBX
    CMP ECX, 0xa
    JGE while_5_end
    MOV EDX, __str_1
    MOV ESI, [EBP-4]
    MOV EDI, [EBP-4]
    PUSH EDI             ; push 1 args for function call
    CALL fibonacci
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-8], EAX
    ADD ESP, 0x4         ; clean up 4 bytes that were pushed
    PUSH [EBP-8]         ; push 3 args for function call
    PUSH ESI
    PUSH EDX
    CALL printf
    ADD ESP, 0xc         ; clean up 12 bytes that were pushed
    JMP while_5_begin
while_5_end:
    MOV [EBP-4], 0x0
while_6_begin:
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-12], [EBP-4]
    MOV EAX, [EBP-12]
    ADD EAX, 0x1
    MOV [EBP-4], EAX
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-16], [EBP-12]
    CMP [EBP-16], 0xa
    JGE while_6_end
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-20], __str_2
    SUB ESP, 0x4         ; grab some space for temp register
    MOV E(unknown), [EBP-4]
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-28], [EBP-4]
    PUSH [EBP-28]        ; push 1 args for function call
    CALL factorial
    SUB ESP, 0x4         ; grab some space for temp register
    MOV [EBP-32], EAX
    ADD ESP, 0x4         ; clean up 4 bytes that were pushed
    PUSH [EBP-32]        ; push 3 args for function call
    PUSH E(unknown)
    PUSH [EBP-20]
    CALL printf
    ADD ESP, 0xc         ; clean up 12 bytes that were pushed
    JMP while_6_begin
math_demo_exit:
    MOV ESP, EBP         ; tear down stack frame
    POP EBP
    RET

nested_loops_test:
    PUSH EBP             ; establish stack frame
    MOV EBP, ESP
    SUB ESP, 0x8         ; space for local vars
    ; [EBP -4] local var "outer", 4 bytes
    ; [EBP -8] local var "inner", 4 bytes
    MOV [EBP-4], 0xa
while_7_begin:
    CMP [EBP-4], 0x0
    JLE while_7_end
    MOV [EBP-8], 0xf
while_8_begin:
    CMP [EBP-8], 0x0
    JLE while_8_end
    MOV EBX, [EBP-8]
    MOV EAX, EBX
    SUB EAX, 0x1
    MOV [EBP-8], EAX
    JMP while_8_begin
while_8_end:
    MOV ECX, [EBP-4]
    MOV EAX, ECX
    SUB EAX, 0x1
    MOV [EBP-4], EAX
    JMP while_7_begin
nested_loops_test_exit:
    MOV ESP, EBP         ; tear down stack frame
    POP EBP
    RET

test_pre_post_inc_dec:
    PUSH EBP             ; establish stack frame
    MOV EBP, ESP
    SUB ESP, 0x14        ; space for local vars
    ; [EBP -4] local var "a", 4 bytes
    ; [EBP -8] local var "b", 4 bytes
    ; [EBP-12] local var "c", 4 bytes
    ; [EBP-16] local var "d", 4 bytes
    ; [EBP-20] local var "result", 4 bytes
    MOV [EBP-4], 0x8
    MOV EBX, [EBP-4]
    MOV EAX, EBX
    ADD EAX, 0x1
    MOV [EBP-4], EAX
    MOV [EBP-20], EBX
    MOV [EBP-8], 0x8
    MOV ECX, [EBP-8]
    MOV EAX, ECX
    ADD EAX, 0x1
    MOV [EBP-8], EAX
    MOV [EBP-20], [EBP-8]
    MOV [EBP-12], 0x8
    MOV EDX, [EBP-12]
    MOV EAX, EDX
    SUB EAX, 0x1
    MOV [EBP-12], EAX
    MOV [EBP-20], EDX
    MOV [EBP-16], 0x8
    MOV ESI, [EBP-16]
    MOV EAX, ESI
    SUB EAX, 0x1
    MOV [EBP-16], EAX
    MOV [EBP-20], [EBP-16]
test_pre_post_inc_dec_exit:
    MOV ESP, EBP         ; tear down stack frame
    POP EBP
    RET

main:
    PUSH EBP             ; establish stack frame
    MOV EBP, ESP
    SUB ESP, 0x1d        ; space for local vars
    ; [EBP +8] argument "argc", 4 bytes
    ; [EBP+12] argument "argv", 4 bytes
    ; [EBP -4] local var "a", 4 bytes
    ; [EBP -8] local var "b", 4 bytes
    ; [EBP -9] local var "c", 1 bytes
    ; [EBP-13] local var "d", 4 bytes
    ; [EBP-29] local var "buffer", 16 bytes
    MOV [EBP-4], 0x1
    MOV EBX, [EBP-4]
    MOV ECX, 0x2
    MOV EAX, EBX
    ADD EAX, ECX
    MOV [EBP-8], EAX
    CALL math_demo
main_exit:
    MOV ESP, EBP         ; tear down stack frame
    POP EBP
    RET

