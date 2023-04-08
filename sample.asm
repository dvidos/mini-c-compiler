get_next_counter_value:
    PUSH BP                  ; establish stack frame
    MOV  BP, SP
    ; BX allocated to r1
    MOV  BX, counter
    ; CX allocated to r2
    MOV  CX, 0x1
    MOV  AX, BX              ; IR: counter = r1 + r2
    ADD  AX, CX
    MOV  counter, AX
    ; BX released from r1
    ; CX released from r2
    ; BX allocated to r3
    MOV  BX, counter
    MOV  AX, BX              ; IR: return r3
    JMP  get_next_counter_value_exit
    ; BX released from r3
get_next_counter_value_exit:
    MOV  SP, BP              ; tear down stack frame
    POP  BP
    RET  


rect_area:
    PUSH BP                  ; establish stack frame
    MOV  BP, SP
    ; [EBP +8] argument "width", 4 bytes
    ; [EBP+12] argument "height", 4 bytes
    ; BX allocated to r5
    MOV  BX, [BP+8]
    ; CX allocated to r6
    MOV  CX, [BP+12]
    ; DX allocated to r4
    MOV  AX, BX
    MUL  AX, CX
    MOV  DX, AX
    ; BX released from r5
    ; CX released from r6
    MOV  AX, DX              ; IR: return r4
    JMP  rect_area_exit
    ; DX released from r4
rect_area_exit:
    MOV  SP, BP              ; tear down stack frame
    POP  BP
    RET  


triangle_area:
    PUSH BP                  ; establish stack frame
    MOV  BP, SP
    ; [EBP +8] argument "width", 4 bytes
    ; [EBP+12] argument "height", 4 bytes
    ; BX allocated to r8
    MOV  BX, [BP+8]
    ; CX allocated to r10
    MOV  CX, [BP+12]
    ; DX allocated to r11
    MOV  DX, 0x2
    ; SI allocated to r9
    MOV  AX, CX
    DIV  AX, DX
    MOV  SI, AX
    ; CX released from r10
    ; DX released from r11
    ; CX allocated to r7
    MOV  AX, BX
    MUL  AX, SI
    MOV  CX, AX
    ; BX released from r8
    ; SI released from r9
    MOV  AX, CX              ; IR: return r7
    JMP  triangle_area_exit
    ; CX released from r7
triangle_area_exit:
    MOV  SP, BP              ; tear down stack frame
    POP  BP
    RET  


circle_area:
    PUSH BP                  ; establish stack frame
    MOV  BP, SP
    ; [EBP +8] argument "radius", 4 bytes
    ; BX allocated to r13
    MOV  BX, 0x3
    ; CX allocated to r15
    MOV  CX, [BP+8]
    ; DX allocated to r16
    MOV  DX, [BP+8]
    ; SI allocated to r14
    MOV  AX, CX
    MUL  AX, DX
    MOV  SI, AX
    ; CX released from r15
    ; DX released from r16
    ; CX allocated to r12
    MOV  AX, BX
    MUL  AX, SI
    MOV  CX, AX
    ; BX released from r13
    ; SI released from r14
    MOV  AX, CX              ; IR: return r12
    JMP  circle_area_exit
    ; CX released from r12
circle_area_exit:
    MOV  SP, BP              ; tear down stack frame
    POP  BP
    RET  


fibonacci:
    PUSH BP                  ; establish stack frame
    MOV  BP, SP
    ; [EBP +8] argument "n", 4 bytes
    CMP  [BP+8], 0x2         ; IR: if n > 2 goto if_3_end
    JAB  if_3_end
    ; BX allocated to r17
    MOV  BX, [BP+8]
    MOV  AX, BX              ; IR: return r17
    JMP  fibonacci_exit
    ; BX released from r17
if_3_end:
    ; BX allocated to r21
    MOV  BX, [BP+8]
    ; CX allocated to r22
    MOV  CX, 0x1
    ; DX allocated to r20
    MOV  AX, BX
    SUB  AX, CX
    MOV  DX, AX
    ; BX released from r21
    ; CX released from r22
    ; BX allocated to r19
    PUSH DX
    CALL fibonacci
    MOV  BX, AX              ; grab returned value
    ADD  SP, 0x4             ; clean up 4 bytes that were pushed as arguments
    ; DX released from r20
    ; CX allocated to r25
    MOV  CX, [BP+8]
    ; DX allocated to r26
    MOV  DX, 0x2
    ; SI allocated to r24
    MOV  AX, CX
    SUB  AX, DX
    MOV  SI, AX
    ; CX released from r25
    ; DX released from r26
    ; CX allocated to r23
    PUSH SI
    CALL fibonacci
    MOV  CX, AX              ; grab returned value
    ADD  SP, 0x4             ; clean up 4 bytes that were pushed as arguments
    ; SI released from r24
    ; DX allocated to r18
    MOV  AX, BX
    ADD  AX, CX
    MOV  DX, AX
    ; BX released from r19
    ; CX released from r23
    MOV  AX, DX              ; IR: return r18
    JMP  fibonacci_exit
    ; DX released from r18
fibonacci_exit:
    MOV  SP, BP              ; tear down stack frame
    POP  BP
    RET  


factorial:
    PUSH BP                  ; establish stack frame
    MOV  BP, SP
    ; [EBP +8] argument "n", 4 bytes
    CMP  [BP+8], 0x1         ; IR: if n > 1 goto if_4_end
    JAB  if_4_end
    ; BX allocated to r27
    MOV  BX, [BP+8]
    MOV  AX, BX              ; IR: return r27
    JMP  factorial_exit
    ; BX released from r27
if_4_end:
    ; BX allocated to r29
    MOV  BX, [BP+8]
    ; CX allocated to r32
    MOV  CX, [BP+8]
    ; DX allocated to r33
    MOV  DX, 0x1
    ; SI allocated to r31
    MOV  AX, CX
    SUB  AX, DX
    MOV  SI, AX
    ; CX released from r32
    ; DX released from r33
    ; CX allocated to r30
    PUSH SI
    CALL factorial
    MOV  CX, AX              ; grab returned value
    ADD  SP, 0x4             ; clean up 4 bytes that were pushed as arguments
    ; SI released from r31
    ; DX allocated to r28
    MOV  AX, BX
    MUL  AX, CX
    MOV  DX, AX
    ; BX released from r29
    ; CX released from r30
    MOV  AX, DX              ; IR: return r28
    JMP  factorial_exit
    ; DX released from r28
factorial_exit:
    MOV  SP, BP              ; tear down stack frame
    POP  BP
    RET  


math_demo:
    PUSH BP                  ; establish stack frame
    MOV  BP, SP
    SUB  SP, 0x4             ; reserve 4 bytes for local vars
    ; [EBP -4] local var "i", 4 bytes
    MOV  [BP-4], 0x0         ; IR: i = 0
while_5_begin:
    ; BX allocated to r35
    MOV  BX, [BP-4]
    MOV  AX, BX              ; IR: i = r35 + 1
    ADD  AX, 0x1
    MOV  [BP-4], AX
    ; CX allocated to r34
    MOV  CX, BX
    ; BX released from r35
    CMP  CX, 0xa             ; IR: if r34 >= 0xa goto while_5_end
    JAE  while_5_end
    ; CX released from r34
    ; BX released from r34
    ; BX allocated to r36
    MOV  BX, __str_1
    ; CX allocated to r37
    MOV  CX, [BP-4]
    ; DX allocated to r39
    MOV  DX, [BP-4]
    ; SI allocated to r38
    PUSH DX
    CALL fibonacci
    MOV  SI, AX              ; grab returned value
    ADD  SP, 0x4             ; clean up 4 bytes that were pushed as arguments
    ; DX released from r39
    PUSH SI                  ; IR: call printf passing r36, r37, r38
    PUSH CX
    PUSH BX
    CALL printf
    ADD  SP, 0xc             ; clean up 12 bytes that were pushed as arguments
    ; BX released from r36
    ; CX released from r37
    ; SI released from r38
    JMP  while_5_begin       ; IR: goto while_5_begin
while_5_end:
    MOV  [BP-4], 0x0         ; IR: i = 0
while_6_begin:
    ; BX allocated to r41
    MOV  BX, [BP-4]
    MOV  AX, BX              ; IR: i = r41 + 1
    ADD  AX, 0x1
    MOV  [BP-4], AX
    ; CX allocated to r40
    MOV  CX, BX
    ; BX released from r41
    CMP  CX, 0xa             ; IR: if r40 >= 0xa goto while_6_end
    JAE  while_6_end
    ; CX released from r40
    ; BX released from r40
    ; BX allocated to r42
    MOV  BX, __str_2
    ; CX allocated to r43
    MOV  CX, [BP-4]
    ; DX allocated to r45
    MOV  DX, [BP-4]
    ; SI allocated to r44
    PUSH DX
    CALL factorial
    MOV  SI, AX              ; grab returned value
    ADD  SP, 0x4             ; clean up 4 bytes that were pushed as arguments
    ; DX released from r45
    PUSH SI                  ; IR: call printf passing r42, r43, r44
    PUSH CX
    PUSH BX
    CALL printf
    ADD  SP, 0xc             ; clean up 12 bytes that were pushed as arguments
    ; BX released from r42
    ; CX released from r43
    ; SI released from r44
    JMP  while_6_begin       ; IR: goto while_6_begin
math_demo_exit:
    MOV  SP, BP              ; tear down stack frame
    POP  BP
    RET  


nested_loops_test:
    PUSH BP                  ; establish stack frame
    MOV  BP, SP
    SUB  SP, 0x8             ; reserve 8 bytes for local vars
    ; [EBP -4] local var "outer", 4 bytes
    ; [EBP -8] local var "inner", 4 bytes
    MOV  [BP-4], 0xa         ; IR: outer = 0xa
while_7_begin:
    CMP  [BP-4], 0x0         ; IR: if outer <= 0 goto while_7_end
    JBE  while_7_end
    MOV  [BP-8], 0xf         ; IR: inner = 0xf
while_8_begin:
    CMP  [BP-8], 0x0         ; IR: if inner <= 0 goto while_8_end
    JBE  while_8_end
    ; BX allocated to r46
    MOV  BX, [BP-8]
    MOV  AX, BX              ; IR: inner = r46 - 1
    SUB  AX, 0x1
    MOV  [BP-8], AX
    ; BX released from r46
    JMP  while_8_begin       ; IR: goto while_8_begin
while_8_end:
    ; BX allocated to r47
    MOV  BX, [BP-4]
    MOV  AX, BX              ; IR: outer = r47 - 1
    SUB  AX, 0x1
    MOV  [BP-4], AX
    ; BX released from r47
    JMP  while_7_begin       ; IR: goto while_7_begin
nested_loops_test_exit:
    MOV  SP, BP              ; tear down stack frame
    POP  BP
    RET  


test_pre_post_inc_dec:
    PUSH BP                  ; establish stack frame
    MOV  BP, SP
    SUB  SP, 0x14            ; reserve 20 bytes for local vars
    ; [EBP -4] local var "a", 4 bytes
    ; [EBP -8] local var "b", 4 bytes
    ; [EBP-12] local var "c", 4 bytes
    ; [EBP-16] local var "d", 4 bytes
    ; [EBP-20] local var "result", 4 bytes
    MOV  [BP-4], 0x8         ; IR: a = 8
    ; BX allocated to r48
    MOV  BX, [BP-4]
    MOV  AX, BX              ; IR: a = r48 + 1
    ADD  AX, 0x1
    MOV  [BP-4], AX
    MOV  [BP-20], BX         ; IR: result = r48
    ; BX released from r48
    MOV  [BP-8], 0x8         ; IR: b = 8
    ; BX allocated to r49
    MOV  BX, [BP-8]
    MOV  AX, BX              ; IR: b = r49 + 1
    ADD  AX, 0x1
    MOV  [BP-8], AX
    ; BX released from r49
    MOV  AX, [BP-8]          ; IR: result = b
    MOV  [BP-20], AX
    MOV  [BP-12], 0x8        ; IR: c = 8
    ; BX allocated to r50
    MOV  BX, [BP-12]
    MOV  AX, BX              ; IR: c = r50 - 1
    SUB  AX, 0x1
    MOV  [BP-12], AX
    MOV  [BP-20], BX         ; IR: result = r50
    ; BX released from r50
    MOV  [BP-16], 0x8        ; IR: d = 8
    ; BX allocated to r51
    MOV  BX, [BP-16]
    MOV  AX, BX              ; IR: d = r51 - 1
    SUB  AX, 0x1
    MOV  [BP-16], AX
    ; BX released from r51
    MOV  AX, [BP-16]         ; IR: result = d
    MOV  [BP-20], AX
test_pre_post_inc_dec_exit:
    MOV  SP, BP              ; tear down stack frame
    POP  BP
    RET  


main:
    PUSH BP                  ; establish stack frame
    MOV  BP, SP
    SUB  SP, 0x1d            ; reserve 29 bytes for local vars
    ; [EBP +8] argument "argc", 4 bytes
    ; [EBP+12] argument "argv", 4 bytes
    ; [EBP -4] local var "a", 4 bytes
    ; [EBP -8] local var "b", 4 bytes
    ; [EBP -9] local var "c", 1 bytes
    ; [EBP-13] local var "d", 4 bytes
    ; [EBP-29] local var "buffer", 16 bytes
    MOV  [BP-4], 0x1         ; IR: a = 1
    ; BX allocated to r52
    MOV  BX, [BP-4]
    ; CX allocated to r53
    MOV  CX, 0x2
    MOV  AX, BX              ; IR: b = r52 + r53
    ADD  AX, CX
    MOV  [BP-8], AX
    ; BX released from r52
    CALL math_demo           ; IR: call math_demo
main_exit:
    MOV  SP, BP              ; tear down stack frame
    POP  BP
    RET  


