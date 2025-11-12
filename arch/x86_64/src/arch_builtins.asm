bits 64

extern cpufeat_level
extern test_cpufeats
test_cpufeats:
	push rax
	push rbx
	push rcx
	push rdx
	mov rax, 0x01
	cpuid
	test rdx, 1 << 25
	jz .exit
	add DWORD [cpufeat_level], 1

	test rcx, 1 << 28
	jz .exit
	add DWORD [cpufeat_level], 1

	mov rax, 7
	xor rcx, rcx
	cpuid
	test rbx, 1 << 16
	jz .exit
	add DWORD [cpufeat_level], 1
.exit:
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret
