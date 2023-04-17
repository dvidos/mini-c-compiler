_start:
    nop
    call func1
    nop
label1:
    nop
    jmp label1
    nop
    jmp func1
    nop
func1:
    mov ax, 1
    mov bx, 2
    mov cx, 3
    ret

