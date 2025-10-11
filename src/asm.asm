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

section .data

align 16
empty_stack: times 0xfff0 db 0
align 4

register_save:
rax_save: dq 0
rbx_save: dq 0
rcx_save: dq 0
rdx_save: dq 0
rsp_save: dq empty_stack + 0xfff0
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

align 16
fpu_save: times 4096 db 0
align 4

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

align 16
caller_fpu_save: times 4096 db 0
align 4

return_point: dq 0
context_switching: dd 0

align 16
fpu_temp: dq 0.0
align 4

section .text

; close your eyes
asm_resume:
	mov DWORD [context_switching], 1
	mov QWORD [caller_rax_save], rax
	mov QWORD [caller_rbx_save], rbx
	mov QWORD [caller_rcx_save], rcx
	mov QWORD [caller_rdx_save], rdx
	mov QWORD [caller_rsp_save], rsp
	mov QWORD [caller_rbp_save], rbp
	mov QWORD [caller_rsi_save], rsi
	mov QWORD [caller_rdi_save], rdi
	mov QWORD [caller_r8_save], r8
	mov QWORD [caller_r9_save], r9
	mov QWORD [caller_r10_save], r10
	mov QWORD [caller_r11_save], r11
	mov QWORD [caller_r12_save], r12
	mov QWORD [caller_r13_save], r13
	mov QWORD [caller_r14_save], r14
	mov QWORD [caller_r15_save], r15

	pushfq
	pop rax
	mov QWORD [caller_rflags_save], rax

	xor rax, rax
	xor rdx, rdx
	fxsave [caller_fpu_save]

	fxrstor [fpu_save]
	mov rax, [rflags_save]
	push rax
	popfq

	mov rax, [rax_save]
	mov rbx, [rbx_save]
	mov rcx, [rcx_save]
	mov rdx, [rdx_save]
	mov rsp, [rsp_save]
	mov rbp, [rbp_save]
	mov rsi, [rsi_save]
	mov rdi, [rdi_save]
	mov r8, [r8_save]
	mov r9, [r9_save]
	mov r10, [r10_save]
	mov r11, [r11_save]
	mov r12, [r12_save]
	mov r13, [r13_save]
	mov r14, [r14_save]
	mov r15, [r15_save]

	jmp [return_point]

asm_continue:
	mov QWORD [rax_save], rax
	mov QWORD [rbx_save], rbx
	mov QWORD [rcx_save], rcx
	mov QWORD [rdx_save], rdx
	mov QWORD [rsp_save], rsp
	mov QWORD [rbp_save], rbp
	mov QWORD [rsi_save], rsi
	mov QWORD [rdi_save], rdi
	mov QWORD [r8_save], r8
	mov QWORD [r9_save], r9
	mov QWORD [r10_save], r10
	mov QWORD [r11_save], r11
	mov QWORD [r12_save], r12
	mov QWORD [r13_save], r13
	mov QWORD [r14_save], r14
	mov QWORD [r15_save], r15

	mov rsp, [caller_rsp_save]

	pushfq
	pop rax
	mov QWORD [rflags_save], rax

	xor rax, rax
	xor rdx, rdx
	fxsave [fpu_save]

reload_state:
	mov rax, [caller_rflags_save]
	push rax
	popfq

	fxrstor [caller_fpu_save]

	mov rax, [caller_rax_save]
	mov rbx, [caller_rbx_save]
	mov rcx, [caller_rcx_save]
	mov rdx, [caller_rdx_save]
	mov rsp, [caller_rsp_save]
	mov rbp, [caller_rbp_save]
	mov rsi, [caller_rsi_save]
	mov rdi, [caller_rdi_save]
	mov r8, [caller_r8_save]
	mov r9, [caller_r9_save]
	mov r10, [caller_r10_save]
	mov r11, [caller_r11_save]
	mov r12, [caller_r12_save]
	mov r13, [caller_r13_save]
	mov r14, [caller_r14_save]
	mov r15, [caller_r15_save]
	mov DWORD [context_switching], 0
	ret

fpu_float_to_double:
	fld TWORD [rdi]
	fstp QWORD [fpu_temp]
	movsd xmm0, [fpu_temp]
	ret

setup_fpu:
	xor rax, rax
	xor rdx, rdx
	fxsave [caller_fpu_save]
	finit
	xor rax, rax
	xor rdx, rdx
	pxor xmm0, xmm0
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
	fxsave [fpu_save]
	fxrstor [caller_fpu_save]
	ret
