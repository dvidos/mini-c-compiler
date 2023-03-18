get_next_counter_value:
    PUSH EBP
    MOV EBP, ESP
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
    JMP triangle_area_end
triangle_area_end:
    MOV ESP, EBP
    POP EBP
    RET

circle_area:
    PUSH EBP
    MOV EBP, ESP
    ; [EBP +8] argument "radius", 4 bytes
    JMP circle_area_end
circle_area_end:
    MOV ESP, EBP
    POP EBP
    RET

fibonacci:
    PUSH EBP
    MOV EBP, ESP
    ; [EBP +8] argument "n", 4 bytes
    CMP EAX, EDX
    JGT if_3_end
    JMP fibonacci_end
if_3_end:
    JMP fibonacci_end
fibonacci_end:
    MOV ESP, EBP
    POP EBP
    RET

factorial:
    PUSH EBP
    MOV EBP, ESP
    ; [EBP +8] argument "n", 4 bytes
    CMP EAX, EDX
    JGT if_4_end
    JMP factorial_end
if_4_end:
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
while_5_begin:
    CMP EAX, EDX
    JGE while_5_end
    JMP while_5_begin
while_6_begin:
    CMP EAX, EDX
    JGE while_6_end
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
while_7_begin:
    CMP EAX, EDX
    JLE while_7_end
while_8_begin:
    CMP EAX, EDX
    JLE while_8_end
    JMP while_8_begin
while_8_end:
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
main_end:
    MOV ESP, EBP
    POP EBP
    RET

