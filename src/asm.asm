bits 64

extern return_point
extern asm_resume
extern reload_state
extern register_save
extern caller_register_save
extern fpu_save
extern setup_fpu
extern fpu_float_to_double
extern context_switching
extern asm_continue

section .bss

align 32
fpu_save: resb 4096
align 32
caller_fpu_save: resb 4096
align 16
fpu_temp: resq 1

section .data

register_save:
rax_save: dq 0
rbx_save: dq 0
rcx_save: dq 0
rdx_save: dq 0
rsp_save: dq 0
rbp_save: dq 0
rsi_save: dq 0
rdi_save: dq 0
r8_save: dq 0
r9_save: dq 0
r10_save: dq 0
r11_save: dq 0
r12_save: dq 0
r13_save: dq 0
r14_save: dq 0
r15_save: dq 0
rflags_save: dq 0

caller_register_save:
caller_rax_save: dq 0
caller_rbx_save: dq 0
caller_rcx_save: dq 0
caller_rdx_save: dq 0
caller_rsp_save: dq 0
caller_rbp_save: dq 0
caller_rsi_save: dq 0
caller_rdi_save: dq 0
caller_r8_save: dq 0
caller_r9_save: dq 0
caller_r10_save: dq 0
caller_r11_save: dq 0
caller_r12_save: dq 0
caller_r13_save: dq 0
caller_r14_save: dq 0
caller_r15_save: dq 0
caller_rflags_save: dq 0

return_point: dq 0
context_switching: dd 0

xsave_supported: dd 0
fxsave_supported: dd 0

section .text

save_caller_fpu_state:
	mov rax, 0xffffffff
	mov rdx, 0xffffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxsave
	xsave [rel caller_fpu_save]
	ret
.fxsave:
	fxsave [rel caller_fpu_save]
	ret

restore_caller_fpu_state:
	cmp DWORD [rel xsave_supported], 0
	je .fxrstor
	xrstor [rel caller_fpu_save]
	ret
.fxrstor:
	fxrstor [rel caller_fpu_save]
	ret

save_fpu_state:
	mov rax, 0xffffffff
	mov rdx, 0xffffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxsave
	xsave [rel fpu_save]
	ret
.fxsave:
	fxsave [rel fpu_save]
	ret

restore_fpu_state:
	cmp DWORD [rel xsave_supported], 0
	je .fxrstor
	xrstor [rel fpu_save]
	ret
.fxrstor:
	fxrstor [rel fpu_save]
	ret

; close your eyes
asm_resume:
	mov DWORD [rel context_switching], 1
	mov QWORD [rel caller_rax_save], rax
	mov QWORD [rel caller_rbx_save], rbx
	mov QWORD [rel caller_rcx_save], rcx
	mov QWORD [rel caller_rdx_save], rdx
	mov QWORD [rel caller_rsp_save], rsp
	mov QWORD [rel caller_rbp_save], rbp
	mov QWORD [rel caller_rsi_save], rsi
	mov QWORD [rel caller_rdi_save], rdi
	mov QWORD [rel caller_r8_save], r8
	mov QWORD [rel caller_r9_save], r9
	mov QWORD [rel caller_r10_save], r10
	mov QWORD [rel caller_r11_save], r11
	mov QWORD [rel caller_r12_save], r12
	mov QWORD [rel caller_r13_save], r13
	mov QWORD [rel caller_r14_save], r14
	mov QWORD [rel caller_r15_save], r15

	pushfq
	pop rax
	mov QWORD [rel caller_rflags_save], rax

	call save_caller_fpu_state

	call restore_fpu_state
	mov rax, [rel rflags_save]
	push rax
	popfq

	mov rax, [rel rax_save]
	mov rbx, [rel rbx_save]
	mov rcx, [rel rcx_save]
	mov rdx, [rel rdx_save]
	mov rsp, [rel rsp_save]
	mov rbp, [rel rbp_save]
	mov rsi, [rel rsi_save]
	mov rdi, [rel rdi_save]
	mov r8, [rel r8_save]
	mov r9, [rel r9_save]
	mov r10, [rel r10_save]
	mov r11, [rel r11_save]
	mov r12, [rel r12_save]
	mov r13, [rel r13_save]
	mov r14, [rel r14_save]
	mov r15, [rel r15_save]

	jmp [rel return_point]

asm_continue:
	mov QWORD [rel rax_save], rax
	mov QWORD [rel rbx_save], rbx
	mov QWORD [rel rcx_save], rcx
	mov QWORD [rel rdx_save], rdx
	mov QWORD [rel rsp_save], rsp
	mov QWORD [rel rbp_save], rbp
	mov QWORD [rel rsi_save], rsi
	mov QWORD [rel rdi_save], rdi
	mov QWORD [rel r8_save], r8
	mov QWORD [rel r9_save], r9
	mov QWORD [rel r10_save], r10
	mov QWORD [rel r11_save], r11
	mov QWORD [rel r12_save], r12
	mov QWORD [rel r13_save], r13
	mov QWORD [rel r14_save], r14
	mov QWORD [rel r15_save], r15

	mov rsp, [rel caller_rsp_save]

	pushfq
	pop rax
	mov QWORD [rel rflags_save], rax

	call save_fpu_state

reload_state:
	mov rax, [rel caller_rflags_save]
	push rax
	popfq

	call restore_caller_fpu_state

	mov rax, [rel caller_rax_save]
	mov rbx, [rel caller_rbx_save]
	mov rcx, [rel caller_rcx_save]
	mov rdx, [rel caller_rdx_save]
	mov rsp, [rel caller_rsp_save]
	mov rbp, [rel caller_rbp_save]
	mov rsi, [rel caller_rsi_save]
	mov rdi, [rel caller_rdi_save]
	mov r8, [rel caller_r8_save]
	mov r9, [rel caller_r9_save]
	mov r10, [rel caller_r10_save]
	mov r11, [rel caller_r11_save]
	mov r12, [rel caller_r12_save]
	mov r13, [rel caller_r13_save]
	mov r14, [rel caller_r14_save]
	mov r15, [rel caller_r15_save]
	mov DWORD [rel context_switching], 0
	ret

fpu_float_to_double:
	fld TWORD [rdi]
	fstp QWORD [rel fpu_temp]
	movsd xmm0, [rel fpu_temp]
	ret

setup_fpu:
	push rax
	push rbx
	push rcx
	push rdx

	mov DWORD [rel fxsave_supported], 1 ; YES

	mov eax, 0x01
	cpuid
	test ecx, 1 << 28
	jz .no_avx
	mov DWORD [rel xsave_supported], 1 ; YES
.no_avx:
	call save_caller_fpu_state
	finit
	vxorpd xmm0, xmm0
	pxor xmm1, xmm1
	pxor xmm2, xmm2
	pxor xmm3, xmm3
	pxor xmm4, xmm4
	pxor xmm5, xmm5
	pxor xmm6, xmm6
	pxor xmm7, xmm7
	pxor xmm8, xmm8
	pxor xmm9, xmm9
	pxor xmm10, xmm10
	pxor xmm11, xmm11
	pxor xmm12, xmm12
	pxor xmm13, xmm13
	pxor xmm14, xmm14
	pxor xmm15, xmm15
	call save_fpu_state
	call restore_caller_fpu_state
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret
