get_next_counter_value:
    PUSH   BP                ; establish stack frame
    MOV    BP <- SP
    ; BX allocated to r1
    MOV    BX <- counter
    ; CX allocated to r2
    MOV    CX <- 0x1
    MOV    AX <- BX          ; counter = r1 + r2
    ADD    AX <- CX
    MOV    counter <- AX
    ; DX allocated to r3
    MOV    DX <- counter
    MOV    AX <- DX          ; return r3
    JMP    get_next_counter_value_exit
get_next_counter_value_exit:
    MOV    SP <- BP          ; tear down stack frame
    POP    BP
    RET                      ; return value should be in EAX


rect_area:
    PUSH   BP                ; establish stack frame
    MOV    BP <- SP
    ; [EBP +8] argument "width", 4 bytes
    ; [EBP+12] argument "height", 4 bytes
    ; BX allocated to r5
    MOV    BX <- [BP+8]
    ; CX allocated to r6
    MOV    CX <- [BP+12]
    ; DX allocated to r4
    MOV    AX <- BX
    IMUL   AX <- CX
    MOV    DX <- AX
    ; r5 storage released
    ; r6 storage released
    MOV    AX <- DX          ; return r4
    JMP    rect_area_exit
    ; r4 storage released
rect_area_exit:
    MOV    SP <- BP          ; tear down stack frame
    POP    BP
    RET                      ; return value should be in EAX


triangle_area:
    PUSH   BP                ; establish stack frame
    MOV    BP <- SP
    ; [EBP +8] argument "width", 4 bytes
    ; [EBP+12] argument "height", 4 bytes
    ; BX allocated to r8
    MOV    BX <- [BP+8]
    ; CX allocated to r10
    MOV    CX <- [BP+12]
    ; DX allocated to r11
    MOV    DX <- 0x2
    ; SI allocated to r9
    MOV    AX <- CX
    IDIV   AX <- DX
    MOV    SI <- AX
    ; DI allocated to r7
    MOV    AX <- BX
    IMUL   AX <- SI
    MOV    DI <- AX
    MOV    AX <- DI          ; return r7
    JMP    triangle_area_exit
    ; r7 storage released
triangle_area_exit:
    MOV    SP <- BP          ; tear down stack frame
    POP    BP
    RET                      ; return value should be in EAX


circle_area:
    PUSH   BP                ; establish stack frame
    MOV    BP <- SP
    ; [EBP +8] argument "radius", 4 bytes
    ; BX allocated to r13
    MOV    BX <- 0x3
    ; CX allocated to r15
    MOV    CX <- [BP+8]
    ; DX allocated to r16
    MOV    DX <- [BP+8]
    ; SI allocated to r14
    MOV    AX <- CX
    IMUL   AX <- DX
    MOV    SI <- AX
    ; DI allocated to r12
    MOV    AX <- BX
    IMUL   AX <- SI
    MOV    DI <- AX
    MOV    AX <- DI          ; return r12
    JMP    circle_area_exit
circle_area_exit:
    MOV    SP <- BP          ; tear down stack frame
    POP    BP
    RET                      ; return value should be in EAX


fibonacci:
    PUSH   BP                ; establish stack frame
    MOV    BP <- SP
    ; [EBP +8] argument "n", 4 bytes
    CMP    [BP+8] <- 0x2     ; if n > 2 goto if_3_end
    JGT    if_3_end
    ; BX allocated to r17
    MOV    BX <- [BP+8]
    MOV    AX <- BX          ; return r17
    JMP    fibonacci_exit
if_3_end:
    ; CX allocated to r21
    MOV    CX <- [BP+8]
    ; DX allocated to r22
    MOV    DX <- 0x1
    ; SI allocated to r20
    MOV    AX <- CX
    SUB    AX <- DX
    MOV    SI <- AX
    PUSH   SI                ; r19 = call fibonacci passing r20
    CALL   fibonacci
    ; DI allocated to r19
    MOV    DI <- AX
    ADD    SP <- 0x4         ; clean up 4 bytes that were pushed as arguments
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-4] (4 bytes) allocated to r25
    MOV    AX <- [BP+8]
    MOV    [BP-4] <- AX
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-8] (4 bytes) allocated to r26
    MOV    [BP-8] <- 0x2
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-12] (4 bytes) allocated to r24
    MOV    AX <- [BP-4]
    SUB    AX <- [BP-8]
    MOV    [BP-12] <- AX
    PUSH   [BP-12]           ; r23 = call fibonacci passing r24
    CALL   fibonacci
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-16] (4 bytes) allocated to r23
    MOV    [BP-16] <- AX
    ADD    SP <- 0x4         ; clean up 4 bytes that were pushed as arguments
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-20] (4 bytes) allocated to r18
    MOV    AX <- DI
    ADD    AX <- [BP-16]
    MOV    [BP-20] <- AX
    MOV    AX <- [BP-20]     ; return r18
    JMP    fibonacci_exit
fibonacci_exit:
    MOV    SP <- BP          ; tear down stack frame
    POP    BP
    RET                      ; return value should be in EAX


factorial:
    PUSH   BP                ; establish stack frame
    MOV    BP <- SP
    ; [EBP +8] argument "n", 4 bytes
    CMP    [BP+8] <- 0x1     ; if n > 1 goto if_4_end
    JGT    if_4_end
    ; BX allocated to r27
    MOV    BX <- [BP+8]
    MOV    AX <- BX          ; return r27
    JMP    factorial_exit
if_4_end:
    ; CX allocated to r29
    MOV    CX <- [BP+8]
    ; DX allocated to r32
    MOV    DX <- [BP+8]
    ; SI allocated to r33
    MOV    SI <- 0x1
    ; DI allocated to r31
    MOV    AX <- DX
    SUB    AX <- SI
    MOV    DI <- AX
    PUSH   DI                ; r30 = call factorial passing r31
    CALL   factorial
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-4] (4 bytes) allocated to r30
    MOV    [BP-4] <- AX
    ADD    SP <- 0x4         ; clean up 4 bytes that were pushed as arguments
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-8] (4 bytes) allocated to r28
    MOV    AX <- CX
    IMUL   AX <- [BP-4]
    MOV    [BP-8] <- AX
    MOV    AX <- [BP-8]      ; return r28
    JMP    factorial_exit
factorial_exit:
    MOV    SP <- BP          ; tear down stack frame
    POP    BP
    RET                      ; return value should be in EAX


math_demo:
    PUSH   BP                ; establish stack frame
    MOV    BP <- SP
    SUB    SP <- 0x4         ; reserve 4 bytes for local vars
    ; [EBP -4] local var "i", 4 bytes
    MOV    [BP-4] <- 0x0     ; i = 0
while_5_begin:
    ; BX allocated to r35
    MOV    BX <- [BP-4]
    MOV    AX <- BX          ; i = r35 + 1
    ADD    AX <- 0x1
    MOV    [BP-4] <- AX
    ; CX allocated to r34
    MOV    CX <- BX
    CMP    CX <- 0xa         ; if r34 >= 0xa goto while_5_end
    JGE    while_5_end
    ; DX allocated to r36
    MOV    DX <- __str_1
    ; SI allocated to r37
    MOV    SI <- [BP-4]
    ; DI allocated to r39
    MOV    DI <- [BP-4]
    PUSH   DI                ; r38 = call fibonacci passing r39
    CALL   fibonacci
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-8] (4 bytes) allocated to r38
    MOV    [BP-8] <- AX
    ADD    SP <- 0x4         ; clean up 4 bytes that were pushed as arguments
    PUSH   [BP-8]            ; call printf passing r36, r37, r38
    PUSH   SI
    PUSH   DX
    CALL   printf
    ADD    SP <- 0xc         ; clean up 12 bytes that were pushed as arguments
    JMP    while_5_begin
while_5_end:
    MOV    [BP-4] <- 0x0     ; i = 0
while_6_begin:
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-12] (4 bytes) allocated to r41
    MOV    AX <- [BP-4]
    MOV    [BP-12] <- AX
    MOV    AX <- [BP-12]     ; i = r41 + 1
    ADD    AX <- 0x1
    MOV    [BP-4] <- AX
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-16] (4 bytes) allocated to r40
    MOV    AX <- [BP-12]
    MOV    [BP-16] <- AX
    CMP    [BP-16] <- 0xa    ; if r40 >= 0xa goto while_6_end
    JGE    while_6_end
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-20] (4 bytes) allocated to r42
    MOV    AX <- __str_2
    MOV    [BP-20] <- AX
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-24] (4 bytes) allocated to r43
    MOV    AX <- [BP-4]
    MOV    [BP-24] <- AX
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-28] (4 bytes) allocated to r45
    MOV    AX <- [BP-4]
    MOV    [BP-28] <- AX
    PUSH   [BP-28]           ; r44 = call factorial passing r45
    CALL   factorial
    SUB    0x4 <- SP         ; grab some space for temp register
    ; [BP-32] (4 bytes) allocated to r44
    MOV    [BP-32] <- AX
    ADD    SP <- 0x4         ; clean up 4 bytes that were pushed as arguments
    PUSH   [BP-32]           ; call printf passing r42, r43, r44
    PUSH   [BP-24]
    PUSH   [BP-20]
    CALL   printf
    ADD    SP <- 0xc         ; clean up 12 bytes that were pushed as arguments
    JMP    while_6_begin
math_demo_exit:
    MOV    SP <- BP          ; tear down stack frame
    POP    BP
    RET                      ; return value should be in EAX


nested_loops_test:
    PUSH   BP                ; establish stack frame
    MOV    BP <- SP
    SUB    SP <- 0x8         ; reserve 8 bytes for local vars
    ; [EBP -4] local var "outer", 4 bytes
    ; [EBP -8] local var "inner", 4 bytes
    MOV    [BP-4] <- 0xa     ; outer = 0xa
while_7_begin:
    CMP    [BP-4] <- 0x0     ; if outer <= 0 goto while_7_end
    JLE    while_7_end
    MOV    [BP-8] <- 0xf     ; inner = 0xf
while_8_begin:
    CMP    [BP-8] <- 0x0     ; if inner <= 0 goto while_8_end
    JLE    while_8_end
    ; BX allocated to r46
    MOV    BX <- [BP-8]
    MOV    AX <- BX          ; inner = r46 - 1
    SUB    AX <- 0x1
    MOV    [BP-8] <- AX
    JMP    while_8_begin
while_8_end:
    ; CX allocated to r47
    MOV    CX <- [BP-4]
    MOV    AX <- CX          ; outer = r47 - 1
    SUB    AX <- 0x1
    MOV    [BP-4] <- AX
    JMP    while_7_begin
nested_loops_test_exit:
    MOV    SP <- BP          ; tear down stack frame
    POP    BP
    RET                      ; return value should be in EAX


test_pre_post_inc_dec:
    PUSH   BP                ; establish stack frame
    MOV    BP <- SP
    SUB    SP <- 0x14        ; reserve 20 bytes for local vars
    ; [EBP -4] local var "a", 4 bytes
    ; [EBP -8] local var "b", 4 bytes
    ; [EBP-12] local var "c", 4 bytes
    ; [EBP-16] local var "d", 4 bytes
    ; [EBP-20] local var "result", 4 bytes
    MOV    [BP-4] <- 0x8     ; a = 8
    ; BX allocated to r48
    MOV    BX <- [BP-4]
    MOV    AX <- BX          ; a = r48 + 1
    ADD    AX <- 0x1
    MOV    [BP-4] <- AX
    MOV    [BP-20] <- BX     ; result = r48
    MOV    [BP-8] <- 0x8     ; b = 8
    ; CX allocated to r49
    MOV    CX <- [BP-8]
    MOV    AX <- CX          ; b = r49 + 1
    ADD    AX <- 0x1
    MOV    [BP-8] <- AX
    MOV    AX <- [BP-8]      ; result = b
    MOV    [BP-20] <- AX
    MOV    [BP-12] <- 0x8    ; c = 8
    ; DX allocated to r50
    MOV    DX <- [BP-12]
    MOV    AX <- DX          ; c = r50 - 1
    SUB    AX <- 0x1
    MOV    [BP-12] <- AX
    MOV    [BP-20] <- DX     ; result = r50
    MOV    [BP-16] <- 0x8    ; d = 8
    ; SI allocated to r51
    MOV    SI <- [BP-16]
    MOV    AX <- SI          ; d = r51 - 1
    SUB    AX <- 0x1
    MOV    [BP-16] <- AX
    MOV    AX <- [BP-16]     ; result = d
    MOV    [BP-20] <- AX
test_pre_post_inc_dec_exit:
    MOV    SP <- BP          ; tear down stack frame
    POP    BP
    RET                      ; return value should be in EAX


main:
    PUSH   BP                ; establish stack frame
    MOV    BP <- SP
    SUB    SP <- 0x1d        ; reserve 29 bytes for local vars
    ; [EBP +8] argument "argc", 4 bytes
    ; [EBP+12] argument "argv", 4 bytes
    ; [EBP -4] local var "a", 4 bytes
    ; [EBP -8] local var "b", 4 bytes
    ; [EBP -9] local var "c", 1 bytes
    ; [EBP-13] local var "d", 4 bytes
    ; [EBP-29] local var "buffer", 16 bytes
    MOV    [BP-4] <- 0x1     ; a = 1
    ; BX allocated to r52
    MOV    BX <- [BP-4]
    ; CX allocated to r53
    MOV    CX <- 0x2
    MOV    AX <- BX          ; b = r52 + r53
    ADD    AX <- CX
    MOV    [BP-8] <- AX
    CALL   math_demo         ; call math_demo
main_exit:
    MOV    SP <- BP          ; tear down stack frame
    POP    BP
    RET                      ; return value should be in EAX


