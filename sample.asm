get_next_counter_value:
    PUSH   BP                     ; establish stack frame
    MOV    BP <- SP
    MOV    BX <- counter
    MOV    CX <- 0x1
    MOV    AX <- BX
    ADD    AX <- CX
    MOV    counter <- AX
    MOV    CX <- counter
    MOV    AX <- CX               ; set returned value
    JMP    get_next_counter_value_exit
get_next_counter_value_exit:
    MOV    SP <- BP               ; tear down stack frame
    POP    BP
    RET                           ; return value should be in EAX


rect_area:
    PUSH   BP                     ; establish stack frame
    MOV    BP <- SP
    ; [EBP +8] argument "width", 4 bytes
    ; [EBP+12] argument "height", 4 bytes
    MOV    BX <- [BP+8]
    MOV    CX <- [BP+12]
    MOV    AX <- BX
    IMUL   AX <- CX
    MOV    DX <- AX
    MOV    AX <- DX               ; set returned value
    JMP    rect_area_exit
rect_area_exit:
    MOV    SP <- BP               ; tear down stack frame
    POP    BP
    RET                           ; return value should be in EAX


triangle_area:
    PUSH   BP                     ; establish stack frame
    MOV    BP <- SP
    ; [EBP +8] argument "width", 4 bytes
    ; [EBP+12] argument "height", 4 bytes
    MOV    BX <- [BP+8]
    MOV    CX <- [BP+12]
    MOV    DX <- 0x2
    MOV    AX <- CX
    IDIV   AX <- DX
    MOV    SI <- AX
    MOV    AX <- BX
    IMUL   AX <- SI
    MOV    DI <- AX
    MOV    AX <- DI               ; set returned value
    JMP    triangle_area_exit
triangle_area_exit:
    MOV    SP <- BP               ; tear down stack frame
    POP    BP
    RET                           ; return value should be in EAX


circle_area:
    PUSH   BP                     ; establish stack frame
    MOV    BP <- SP
    ; [EBP +8] argument "radius", 4 bytes
    MOV    BX <- 0x3
    MOV    CX <- [BP+8]
    MOV    DX <- [BP+8]
    MOV    AX <- CX
    IMUL   AX <- DX
    MOV    SI <- AX
    MOV    AX <- BX
    IMUL   AX <- SI
    MOV    DI <- AX
    MOV    AX <- DI               ; set returned value
    JMP    circle_area_exit
circle_area_exit:
    MOV    SP <- BP               ; tear down stack frame
    POP    BP
    RET                           ; return value should be in EAX


fibonacci:
    PUSH   BP                     ; establish stack frame
    MOV    BP <- SP
    ; [EBP +8] argument "n", 4 bytes
    MOV    [BP+8] <- 0x2
    JGT    if_3_end
    MOV    BX <- [BP+8]
    MOV    AX <- BX               ; set returned value
    JMP    fibonacci_exit
if_3_end:
    MOV    CX <- [BP+8]
    MOV    DX <- 0x1
    MOV    AX <- CX
    SUB    AX <- DX
    MOV    SI <- AX
    PUSH   SI                     ; push 1 args for function call
    CALL   fibonacci
    MOV    DI <- AX               ; get value returned from function
    ADD    SP <- 0x4              ; clean up 4 bytes that were pushed as arguments
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    AX <- [BP+8]           ; bring value to register for assignment
    MOV    [BP-4] <- AX
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    [BP-8] <- 0x2
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    AX <- [BP-4]
    SUB    AX <- [BP-8]
    MOV    [BP-12] <- AX
    PUSH   [BP-12]                ; push 1 args for function call
    CALL   fibonacci
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    [BP-16] <- AX
    ADD    SP <- 0x4              ; clean up 4 bytes that were pushed as arguments
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    AX <- DI
    ADD    AX <- [BP-16]
    MOV    [BP-20] <- AX
    MOV    AX <- [BP-20]          ; set returned value
    JMP    fibonacci_exit
fibonacci_exit:
    MOV    SP <- BP               ; tear down stack frame
    POP    BP
    RET                           ; return value should be in EAX


factorial:
    PUSH   BP                     ; establish stack frame
    MOV    BP <- SP
    ; [EBP +8] argument "n", 4 bytes
    MOV    [BP+8] <- 0x1
    JGT    if_4_end
    MOV    BX <- [BP+8]
    MOV    AX <- BX               ; set returned value
    JMP    factorial_exit
if_4_end:
    MOV    CX <- [BP+8]
    MOV    DX <- [BP+8]
    MOV    SI <- 0x1
    MOV    AX <- DX
    SUB    AX <- SI
    MOV    DI <- AX
    PUSH   DI                     ; push 1 args for function call
    CALL   factorial
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    [BP-4] <- AX
    ADD    SP <- 0x4              ; clean up 4 bytes that were pushed as arguments
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    AX <- CX
    IMUL   AX <- [BP-4]
    MOV    [BP-8] <- AX
    MOV    AX <- [BP-8]           ; set returned value
    JMP    factorial_exit
factorial_exit:
    MOV    SP <- BP               ; tear down stack frame
    POP    BP
    RET                           ; return value should be in EAX


math_demo:
    PUSH   BP                     ; establish stack frame
    MOV    BP <- SP
    SUB    SP <- 0x4              ; reserve space for local vars
    ; [EBP -4] local var "i", 4 bytes
    MOV    [BP-4] <- 0x0
while_5_begin:
    MOV    BX <- [BP-4]
    MOV    AX <- BX
    ADD    AX <- 0x1
    MOV    [BP-4] <- AX
    MOV    CX <- BX
    MOV    CX <- 0xa
    JGE    while_5_end
    MOV    DX <- __str_1
    MOV    SI <- [BP-4]
    MOV    DI <- [BP-4]
    PUSH   DI                     ; push 1 args for function call
    CALL   fibonacci
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    [BP-8] <- AX
    ADD    SP <- 0x4              ; clean up 4 bytes that were pushed as arguments
    PUSH   [BP-8]                 ; push 3 args for function call
    PUSH   SI
    PUSH   DX
    CALL   printf
    ADD    SP <- 0xc              ; clean up 12 bytes that were pushed as arguments
    JMP    while_5_begin
while_5_end:
    MOV    [BP-4] <- 0x0
while_6_begin:
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    AX <- [BP-4]           ; bring value to register for assignment
    MOV    [BP-12] <- AX
    MOV    AX <- [BP-12]
    ADD    AX <- 0x1
    MOV    [BP-4] <- AX
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    AX <- [BP-12]          ; bring value to register for assignment
    MOV    [BP-16] <- AX
    MOV    [BP-16] <- 0xa
    JGE    while_6_end
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    AX <- __str_2
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    AX <- [BP-4]           ; bring value to register for assignment
    MOV    [BP-24] <- AX
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    AX <- [BP-4]           ; bring value to register for assignment
    MOV    [BP-28] <- AX
    PUSH   [BP-28]                ; push 1 args for function call
    CALL   factorial
    SUB    0x4 <- SP              ; grab some space for temp register
    MOV    [BP-32] <- AX
    ADD    SP <- 0x4              ; clean up 4 bytes that were pushed as arguments
    PUSH   [BP-32]                ; push 3 args for function call
    PUSH   [BP-24]
    PUSH   AX
    CALL   printf
    ADD    SP <- 0xc              ; clean up 12 bytes that were pushed as arguments
    JMP    while_6_begin
math_demo_exit:
    MOV    SP <- BP               ; tear down stack frame
    POP    BP
    RET                           ; return value should be in EAX


nested_loops_test:
    PUSH   BP                     ; establish stack frame
    MOV    BP <- SP
    SUB    SP <- 0x8              ; reserve space for local vars
    ; [EBP -4] local var "outer", 4 bytes
    ; [EBP -8] local var "inner", 4 bytes
    MOV    [BP-4] <- 0xa
while_7_begin:
    MOV    [BP-4] <- 0x0
    JLE    while_7_end
    MOV    [BP-8] <- 0xf
while_8_begin:
    MOV    [BP-8] <- 0x0
    JLE    while_8_end
    MOV    BX <- [BP-8]
    MOV    AX <- BX
    SUB    AX <- 0x1
    MOV    [BP-8] <- AX
    JMP    while_8_begin
while_8_end:
    MOV    CX <- [BP-4]
    MOV    AX <- CX
    SUB    AX <- 0x1
    MOV    [BP-4] <- AX
    JMP    while_7_begin
nested_loops_test_exit:
    MOV    SP <- BP               ; tear down stack frame
    POP    BP
    RET                           ; return value should be in EAX


test_pre_post_inc_dec:
    PUSH   BP                     ; establish stack frame
    MOV    BP <- SP
    SUB    SP <- 0x14             ; reserve space for local vars
    ; [EBP -4] local var "a", 4 bytes
    ; [EBP -8] local var "b", 4 bytes
    ; [EBP-12] local var "c", 4 bytes
    ; [EBP-16] local var "d", 4 bytes
    ; [EBP-20] local var "result", 4 bytes
    MOV    [BP-4] <- 0x8
    MOV    BX <- [BP-4]
    MOV    AX <- BX
    ADD    AX <- 0x1
    MOV    [BP-4] <- AX
    MOV    [BP-20] <- BX
    MOV    [BP-8] <- 0x8
    MOV    CX <- [BP-8]
    MOV    AX <- CX
    ADD    AX <- 0x1
    MOV    [BP-8] <- AX
    MOV    AX <- [BP-8]           ; bring value to register for assignment
    MOV    [BP-20] <- AX
    MOV    [BP-12] <- 0x8
    MOV    DX <- [BP-12]
    MOV    AX <- DX
    SUB    AX <- 0x1
    MOV    [BP-12] <- AX
    MOV    [BP-20] <- DX
    MOV    [BP-16] <- 0x8
    MOV    SI <- [BP-16]
    MOV    AX <- SI
    SUB    AX <- 0x1
    MOV    [BP-16] <- AX
    MOV    AX <- [BP-16]          ; bring value to register for assignment
    MOV    [BP-20] <- AX
test_pre_post_inc_dec_exit:
    MOV    SP <- BP               ; tear down stack frame
    POP    BP
    RET                           ; return value should be in EAX


main:
    PUSH   BP                     ; establish stack frame
    MOV    BP <- SP
    SUB    SP <- 0x1d             ; reserve space for local vars
    ; [EBP +8] argument "argc", 4 bytes
    ; [EBP+12] argument "argv", 4 bytes
    ; [EBP -4] local var "a", 4 bytes
    ; [EBP -8] local var "b", 4 bytes
    ; [EBP -9] local var "c", 1 bytes
    ; [EBP-13] local var "d", 4 bytes
    ; [EBP-29] local var "buffer", 16 bytes
    MOV    [BP-4] <- 0x1
    MOV    BX <- [BP-4]
    MOV    CX <- 0x2
    MOV    AX <- BX
    ADD    AX <- CX
    MOV    [BP-8] <- AX
    CALL   math_demo
main_exit:
    MOV    SP <- BP               ; tear down stack frame
    POP    BP
    RET                           ; return value should be in EAX


