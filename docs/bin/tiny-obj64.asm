
tiny-obj64:     file format elf64-x86-64


Disassembly of section .text:

0000000000000000 <main>:
   0:	f3 0f 1e fa          	endbr64 
   4:	55                   	push   rbp
   5:	48 89 e5             	mov    rbp,rsp
   8:	b8 78 56 34 12       	mov    eax,0x12345678
   d:	5d                   	pop    rbp
   e:	c3                   	ret    
