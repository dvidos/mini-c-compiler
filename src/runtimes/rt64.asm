.intel_syntax noprefix
.intel_mnemonic


.global syscall
syscall:
	mov rax, rdi
	mov rdi, rsi  
	mov rsi, rdx
	mov rdx, rcx
	mov rcx, r8
	mov r8, r9
	mov r9, [rsp + 8]
	syscall
	ret

.global _start
_start:
	mov rdi, [rsp]
	lea rsi, [rsp + 8]
	call main

	mov rdi, rax
	mov rax, 60
	syscall


