bits 32

extern cpufeat_level
extern test_cpufeats
test_cpufeats:
	push eax
	push ebx
	push ecx
	push edx
	mov eax, 0x01
	cpuid
	test edx, 1 << 25
	jz .exit
	add DWORD [cpufeat_level], 1

	test ecx, 1 << 28
	jz .exit
	add DWORD [cpufeat_level], 1

	mov eax, 7
	xor ecx, ecx
	cpuid
	test ebx, 1 << 16
	jz .exit
	add DWORD [cpufeat_level], 1
.exit:
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret
