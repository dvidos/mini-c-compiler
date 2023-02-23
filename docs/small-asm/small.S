.intel_syntax noprefix

# see https://www.tutorialspoint.com/assembly_programming/assembly_system_calls.htm

.global _start
_start:
    mov eax, 0x1234

    # exit
    mov eax, 1  # exit() system call
    mov ebx, 8  # return code
    int 0x80    # make sys call
