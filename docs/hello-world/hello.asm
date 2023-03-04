; using this as an example:
; https://www.tutorialspoint.com/assembly_programming/assembly_system_calls.htm

section .data
message db "Hello world!", 0x0a, 0x00

section .text
global _start

_start:
    
    mov eax, 4            ; linux syscall write() eax=func, ebx=handle, ecx=buffer, edx=length
    mov ebx, 1
    mov ecx, message
    mov edx, 13
    int 0x80

    mov eax, 1            ; linux syscall exit() eax=func, ebx=exit_code
    mov ebx, 0
    int 0x80

