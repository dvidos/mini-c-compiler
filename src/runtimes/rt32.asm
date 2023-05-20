.intel_syntax noprefix
.intel_mnemonic



.global syscall
syscall:
	mov eax, edi
	mov edi, esi  
	mov esi, edx
	mov edx, ecx
	mov ecx, [esp + 8]
	syscall
	ret

.global _start
_start:
	mov edi, [esp]
	lea esi, [esp + 8]
	call main

	mov edi, eax
	mov eax, 0x60
	syscall


