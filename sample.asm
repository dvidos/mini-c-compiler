get_next_counter_value:
    PUSH   BP                ; establish stack frame
    MOV    BP <- SP
    ; r1 is now BX
    MOV    BX <- counter
    ; r2 is now CX
    MOV    CX <- 0x1
    MOV    AX <- BX          ; counter = r1 + r2
    ADD    AX <- CX
    MOV    counter <- AX
    ; r2 storage released
    ; r3 is now CX
    MOV    CX <- counter
    MOV    AX <- CX          ; return r3
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
    ; r5 is now BX
    MOV    BX <- [BP+8]
    ; r6 is now CX
    MOV    CX <- [BP+12]
    ; r4 is now DX
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
    ; r8 is now BX
    MOV    BX <- [BP+8]
    ; r10 is now CX
    MOV    CX <- [BP+12]
    ; r11 is now DX
    MOV    DX <- 0x2
    ; r9 is now SI
    MOV    AX <- CX
    IDIV   AX <- DX
    MOV    SI <- AX
    ; r7 is now DI
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
    ; r13 is now BX
    MOV    BX <- 0x3
    ; r15 is now CX
    MOV    CX <- [BP+8]
    ; r16 is now DX
    MOV    DX <- [BP+8]
    ; r14 is now SI
    MOV    AX <- CX
    IMUL   AX <- DX
    MOV    SI <- AX
    ; r12 is now DI
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
    ; r17 is now BX
    MOV    BX <- [BP+8]
    MOV    AX <- BX          ; return r17
    JMP    fibonacci_exit
if_3_end:
    ; r21 is now CX
    MOV    CX <- [BP+8]
    ; r22 is now DX
    MOV    DX <- 0x1
    ; r20 is now SI
    MOV    AX <- CX
    SUB    AX <- DX
    MOV    SI <- AX
    PUSH   SI                ; r19 = call fibonacci passing r20
    CALL   fibonacci
    ; r19 is now DI
    MOV    DI <- AX
    ADD    SP <- 0x4         ; clean up 4 bytes that were pushed as arguments
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r25 is now [BP-4] (4 bytes)
    MOV    AX <- [BP+8]
    MOV    [BP-4] <- AX
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r26 is now [BP-8] (4 bytes)
    MOV    [BP-8] <- 0x2
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r24 is now [BP-12] (4 bytes)
    MOV    AX <- [BP-4]
    SUB    AX <- [BP-8]
    MOV    [BP-12] <- AX
    PUSH   [BP-12]           ; r23 = call fibonacci passing r24
    CALL   fibonacci
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r23 is now [BP-16] (4 bytes)
    MOV    [BP-16] <- AX
    ADD    SP <- 0x4         ; clean up 4 bytes that were pushed as arguments
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r18 is now [BP-20] (4 bytes)
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
    ; r27 is now BX
    MOV    BX <- [BP+8]
    MOV    AX <- BX          ; return r27
    JMP    factorial_exit
if_4_end:
    ; r29 is now CX
    MOV    CX <- [BP+8]
    ; r32 is now DX
    MOV    DX <- [BP+8]
    ; r33 is now SI
    MOV    SI <- 0x1
    ; r31 is now DI
    MOV    AX <- DX
    SUB    AX <- SI
    MOV    DI <- AX
    PUSH   DI                ; r30 = call factorial passing r31
    CALL   factorial
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r30 is now [BP-4] (4 bytes)
    MOV    [BP-4] <- AX
    ADD    SP <- 0x4         ; clean up 4 bytes that were pushed as arguments
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r28 is now [BP-8] (4 bytes)
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
    ; r35 is now BX
    MOV    BX <- [BP-4]
    MOV    AX <- BX          ; i = r35 + 1
    ADD    AX <- 0x1
    MOV    [BP-4] <- AX
    ; r34 is now CX
    MOV    CX <- BX
    CMP    CX <- 0xa         ; if r34 >= 0xa goto while_5_end
    JGE    while_5_end
    ; r36 is now DX
    MOV    DX <- __str_1
    ; r37 is now SI
    MOV    SI <- [BP-4]
    ; r39 is now DI
    MOV    DI <- [BP-4]
    PUSH   DI                ; r38 = call fibonacci passing r39
    CALL   fibonacci
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r38 is now [BP-8] (4 bytes)
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
    ; r41 is now [BP-12] (4 bytes)
    MOV    AX <- [BP-4]
    MOV    [BP-12] <- AX
    MOV    AX <- [BP-12]     ; i = r41 + 1
    ADD    AX <- 0x1
    MOV    [BP-4] <- AX
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r40 is now [BP-16] (4 bytes)
    MOV    AX <- [BP-12]
    MOV    [BP-16] <- AX
    CMP    [BP-16] <- 0xa    ; if r40 >= 0xa goto while_6_end
    JGE    while_6_end
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r42 is now [BP-20] (4 bytes)
    MOV    AX <- __str_2
    MOV    [BP-20] <- AX
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r43 is now [BP-24] (4 bytes)
    MOV    AX <- [BP-4]
    MOV    [BP-24] <- AX
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r45 is now [BP-28] (4 bytes)
    MOV    AX <- [BP-4]
    MOV    [BP-28] <- AX
    PUSH   [BP-28]           ; r44 = call factorial passing r45
    CALL   factorial
    SUB    0x4 <- SP         ; grab some space for temp register
    ; r44 is now [BP-32] (4 bytes)
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
    ; r46 is now BX
    MOV    BX <- [BP-8]
    MOV    AX <- BX          ; inner = r46 - 1
    SUB    AX <- 0x1
    MOV    [BP-8] <- AX
    JMP    while_8_begin
while_8_end:
    ; r47 is now CX
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
    ; r48 is now BX
    MOV    BX <- [BP-4]
    MOV    AX <- BX          ; a = r48 + 1
    ADD    AX <- 0x1
    MOV    [BP-4] <- AX
    MOV    [BP-20] <- BX     ; result = r48
    MOV    [BP-8] <- 0x8     ; b = 8
    ; r49 is now CX
    MOV    CX <- [BP-8]
    MOV    AX <- CX          ; b = r49 + 1
    ADD    AX <- 0x1
    MOV    [BP-8] <- AX
    MOV    AX <- [BP-8]      ; result = b
    MOV    [BP-20] <- AX
    MOV    [BP-12] <- 0x8    ; c = 8
    ; r50 is now DX
    MOV    DX <- [BP-12]
    MOV    AX <- DX          ; c = r50 - 1
    SUB    AX <- 0x1
    MOV    [BP-12] <- AX
    MOV    [BP-20] <- DX     ; result = r50
    MOV    [BP-16] <- 0x8    ; d = 8
    ; r51 is now SI
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
    ; r52 is now BX
    MOV    BX <- [BP-4]
    ; r53 is now CX
    MOV    CX <- 0x2
    MOV    AX <- BX          ; b = r52 + r53
    ADD    AX <- CX
    MOV    [BP-8] <- AX
    CALL   math_demo         ; call math_demo
main_exit:
    MOV    SP <- BP          ; tear down stack frame
    POP    BP
    RET                      ; return value should be in EAX


