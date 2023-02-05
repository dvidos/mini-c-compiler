
tiny-dyn32:     file format elf32-i386


Disassembly of section .init:

00001000 <_init>:
    1000:	f3 0f 1e fb          	endbr32 
    1004:	53                   	push   ebx
    1005:	83 ec 08             	sub    esp,0x8
    1008:	e8 83 00 00 00       	call   1090 <__x86.get_pc_thunk.bx>
    100d:	81 c3 cf 2f 00 00    	add    ebx,0x2fcf
    1013:	8b 83 18 00 00 00    	mov    eax,DWORD PTR [ebx+0x18]
    1019:	85 c0                	test   eax,eax
    101b:	74 02                	je     101f <_init+0x1f>
    101d:	ff d0                	call   eax
    101f:	83 c4 08             	add    esp,0x8
    1022:	5b                   	pop    ebx
    1023:	c3                   	ret    

Disassembly of section .plt:

00001030 <__libc_start_main@plt-0x10>:
    1030:	ff b3 04 00 00 00    	push   DWORD PTR [ebx+0x4]
    1036:	ff a3 08 00 00 00    	jmp    DWORD PTR [ebx+0x8]
    103c:	00 00                	add    BYTE PTR [eax],al
	...

00001040 <__libc_start_main@plt>:
    1040:	ff a3 0c 00 00 00    	jmp    DWORD PTR [ebx+0xc]
    1046:	68 00 00 00 00       	push   0x0
    104b:	e9 e0 ff ff ff       	jmp    1030 <_init+0x30>

Disassembly of section .plt.got:

00001050 <__cxa_finalize@plt>:
    1050:	ff a3 14 00 00 00    	jmp    DWORD PTR [ebx+0x14]
    1056:	66 90                	xchg   ax,ax

Disassembly of section .text:

00001060 <_start>:
    1060:	f3 0f 1e fb          	endbr32 
    1064:	31 ed                	xor    ebp,ebp
    1066:	5e                   	pop    esi
    1067:	89 e1                	mov    ecx,esp
    1069:	83 e4 f0             	and    esp,0xfffffff0
    106c:	50                   	push   eax
    106d:	54                   	push   esp
    106e:	52                   	push   edx
    106f:	e8 18 00 00 00       	call   108c <_start+0x2c>
    1074:	81 c3 68 2f 00 00    	add    ebx,0x2f68
    107a:	6a 00                	push   0x0
    107c:	6a 00                	push   0x0
    107e:	51                   	push   ecx
    107f:	56                   	push   esi
    1080:	ff b3 1c 00 00 00    	push   DWORD PTR [ebx+0x1c]
    1086:	e8 b5 ff ff ff       	call   1040 <__libc_start_main@plt>
    108b:	f4                   	hlt    
    108c:	8b 1c 24             	mov    ebx,DWORD PTR [esp]
    108f:	c3                   	ret    

00001090 <__x86.get_pc_thunk.bx>:
    1090:	8b 1c 24             	mov    ebx,DWORD PTR [esp]
    1093:	c3                   	ret    
    1094:	66 90                	xchg   ax,ax
    1096:	66 90                	xchg   ax,ax
    1098:	66 90                	xchg   ax,ax
    109a:	66 90                	xchg   ax,ax
    109c:	66 90                	xchg   ax,ax
    109e:	66 90                	xchg   ax,ax

000010a0 <deregister_tm_clones>:
    10a0:	e8 e4 00 00 00       	call   1189 <__x86.get_pc_thunk.dx>
    10a5:	81 c2 37 2f 00 00    	add    edx,0x2f37
    10ab:	8d 8a 2c 00 00 00    	lea    ecx,[edx+0x2c]
    10b1:	8d 82 2c 00 00 00    	lea    eax,[edx+0x2c]
    10b7:	39 c8                	cmp    eax,ecx
    10b9:	74 1d                	je     10d8 <deregister_tm_clones+0x38>
    10bb:	8b 82 10 00 00 00    	mov    eax,DWORD PTR [edx+0x10]
    10c1:	85 c0                	test   eax,eax
    10c3:	74 13                	je     10d8 <deregister_tm_clones+0x38>
    10c5:	55                   	push   ebp
    10c6:	89 e5                	mov    ebp,esp
    10c8:	83 ec 14             	sub    esp,0x14
    10cb:	51                   	push   ecx
    10cc:	ff d0                	call   eax
    10ce:	83 c4 10             	add    esp,0x10
    10d1:	c9                   	leave  
    10d2:	c3                   	ret    
    10d3:	8d 74 26 00          	lea    esi,[esi+eiz*1+0x0]
    10d7:	90                   	nop
    10d8:	c3                   	ret    
    10d9:	8d b4 26 00 00 00 00 	lea    esi,[esi+eiz*1+0x0]

000010e0 <register_tm_clones>:
    10e0:	e8 a4 00 00 00       	call   1189 <__x86.get_pc_thunk.dx>
    10e5:	81 c2 f7 2e 00 00    	add    edx,0x2ef7
    10eb:	55                   	push   ebp
    10ec:	89 e5                	mov    ebp,esp
    10ee:	53                   	push   ebx
    10ef:	8d 8a 2c 00 00 00    	lea    ecx,[edx+0x2c]
    10f5:	8d 82 2c 00 00 00    	lea    eax,[edx+0x2c]
    10fb:	83 ec 04             	sub    esp,0x4
    10fe:	29 c8                	sub    eax,ecx
    1100:	89 c3                	mov    ebx,eax
    1102:	c1 e8 1f             	shr    eax,0x1f
    1105:	c1 fb 02             	sar    ebx,0x2
    1108:	01 d8                	add    eax,ebx
    110a:	d1 f8                	sar    eax,1
    110c:	74 14                	je     1122 <register_tm_clones+0x42>
    110e:	8b 92 20 00 00 00    	mov    edx,DWORD PTR [edx+0x20]
    1114:	85 d2                	test   edx,edx
    1116:	74 0a                	je     1122 <register_tm_clones+0x42>
    1118:	83 ec 08             	sub    esp,0x8
    111b:	50                   	push   eax
    111c:	51                   	push   ecx
    111d:	ff d2                	call   edx
    111f:	83 c4 10             	add    esp,0x10
    1122:	8b 5d fc             	mov    ebx,DWORD PTR [ebp-0x4]
    1125:	c9                   	leave  
    1126:	c3                   	ret    
    1127:	8d b4 26 00 00 00 00 	lea    esi,[esi+eiz*1+0x0]
    112e:	66 90                	xchg   ax,ax

00001130 <__do_global_dtors_aux>:
    1130:	f3 0f 1e fb          	endbr32 
    1134:	55                   	push   ebp
    1135:	89 e5                	mov    ebp,esp
    1137:	53                   	push   ebx
    1138:	e8 53 ff ff ff       	call   1090 <__x86.get_pc_thunk.bx>
    113d:	81 c3 9f 2e 00 00    	add    ebx,0x2e9f
    1143:	83 ec 04             	sub    esp,0x4
    1146:	80 bb 2c 00 00 00 00 	cmp    BYTE PTR [ebx+0x2c],0x0
    114d:	75 27                	jne    1176 <__do_global_dtors_aux+0x46>
    114f:	8b 83 14 00 00 00    	mov    eax,DWORD PTR [ebx+0x14]
    1155:	85 c0                	test   eax,eax
    1157:	74 11                	je     116a <__do_global_dtors_aux+0x3a>
    1159:	83 ec 0c             	sub    esp,0xc
    115c:	ff b3 28 00 00 00    	push   DWORD PTR [ebx+0x28]
    1162:	e8 e9 fe ff ff       	call   1050 <__cxa_finalize@plt>
    1167:	83 c4 10             	add    esp,0x10
    116a:	e8 31 ff ff ff       	call   10a0 <deregister_tm_clones>
    116f:	c6 83 2c 00 00 00 01 	mov    BYTE PTR [ebx+0x2c],0x1
    1176:	8b 5d fc             	mov    ebx,DWORD PTR [ebp-0x4]
    1179:	c9                   	leave  
    117a:	c3                   	ret    
    117b:	8d 74 26 00          	lea    esi,[esi+eiz*1+0x0]
    117f:	90                   	nop

00001180 <frame_dummy>:
    1180:	f3 0f 1e fb          	endbr32 
    1184:	e9 57 ff ff ff       	jmp    10e0 <register_tm_clones>

00001189 <__x86.get_pc_thunk.dx>:
    1189:	8b 14 24             	mov    edx,DWORD PTR [esp]
    118c:	c3                   	ret    

0000118d <main>:
    118d:	55                   	push   ebp
    118e:	89 e5                	mov    ebp,esp
    1190:	e8 0c 00 00 00       	call   11a1 <__x86.get_pc_thunk.ax>
    1195:	05 47 2e 00 00       	add    eax,0x2e47
    119a:	b8 78 56 34 12       	mov    eax,0x12345678
    119f:	5d                   	pop    ebp
    11a0:	c3                   	ret    

000011a1 <__x86.get_pc_thunk.ax>:
    11a1:	8b 04 24             	mov    eax,DWORD PTR [esp]
    11a4:	c3                   	ret    

Disassembly of section .fini:

000011a8 <_fini>:
    11a8:	f3 0f 1e fb          	endbr32 
    11ac:	53                   	push   ebx
    11ad:	83 ec 08             	sub    esp,0x8
    11b0:	e8 db fe ff ff       	call   1090 <__x86.get_pc_thunk.bx>
    11b5:	81 c3 27 2e 00 00    	add    ebx,0x2e27
    11bb:	83 c4 08             	add    esp,0x8
    11be:	5b                   	pop    ebx
    11bf:	c3                   	ret    
