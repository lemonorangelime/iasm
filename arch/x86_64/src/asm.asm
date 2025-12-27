bits 64

%macro sym 1-2
	extern %1
	%1: %2
%endmacro

extern memcpy

section .data

sym memcpy_address, dq memcpy
align 16
sym mxcsr_fixup, dd 0x1f80

section .bss

align 64
sym fpu_save, resb 4096
align 64
sym caller_fpu_save, resb 4096
align 64
sym temp_fpu_save, resb 4096
align 16
sym fpu_temp, resq 1

sym register_save
sym rax_save, resq 1
sym rbx_save, resq 1
sym rcx_save, resq 1
sym rdx_save, resq 1
sym rsp_save, resq 1
sym rbp_save, resq 1
sym rsi_save, resq 1
sym rdi_save, resq 1
sym r8_save, resq 1
sym r9_save, resq 1
sym r10_save, resq 1
sym r11_save, resq 1
sym r12_save, resq 1
sym r13_save, resq 1
sym r14_save, resq 1
sym r15_save, resq 1
sym rflags_save, resq 1
sym return_point, resq 1
sym cs_save, resw 1
sym ds_save, resw 1
sym ss_save, resw 1
sym es_save, resw 1
sym fs_save, resw 1
sym gs_save, resw 1

sym caller_register_save
sym caller_rax_save, resq 1
sym caller_rbx_save, resq 1
sym caller_rcx_save, resq 1
sym caller_rdx_save, resq 1
sym caller_rsp_save, resq 1
sym caller_rbp_save, resq 1
sym caller_rsi_save, resq 1
sym caller_rdi_save, resq 1
sym caller_r8_save, resq 1
sym caller_r9_save, resq 1
sym caller_r10_save, resq 1
sym caller_r11_save, resq 1
sym caller_r12_save, resq 1
sym caller_r13_save, resq 1
sym caller_r14_save, resq 1
sym caller_r15_save, resq 1
sym caller_rflags_save, resq 1
sym caller_cs_save, resw 1
sym caller_ds_save, resw 1
sym caller_ss_save, resw 1
sym caller_es_save, resw 1
sym caller_fs_save, resw 1
sym caller_gs_save, resw 1

sym temp_register_save
sym temp_rax_save, resq 1
sym temp_rbx_save, resq 1
sym temp_rcx_save, resq 1
sym temp_rdx_save, resq 1
sym temp_rsp_save, resq 1
sym temp_rbp_save, resq 1
sym temp_rsi_save, resq 1
sym temp_rdi_save, resq 1
sym temp_r8_save, resq 1
sym temp_r9_save, resq 1
sym temp_r10_save, resq 1
sym temp_r11_save, resq 1
sym temp_r12_save, resq 1
sym temp_r13_save, resq 1
sym temp_r14_save, resq 1
sym temp_r15_save, resq 1
sym temp_rflags_save, resq 1

sym context_switching, resd 1
sym temp_context_switching, resd 1

sym xsave_supported, resd 1
sym fxsave_supported, resd 1

section .text

sym save_caller_fpu_state
	mov eax, 0x7fffffff
	mov edx, 0x7fffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxsave
	xsave [rel caller_fpu_save]
	ret
.fxsave:
	fxsave [rel caller_fpu_save]
	ret

sym restore_caller_fpu_state
	mov eax, 0x7fffffff
	mov edx, 0x7fffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxrstor
	xrstor [rel caller_fpu_save]
	ret
.fxrstor:
	fxrstor [rel caller_fpu_save]
	ret

sym save_temp_fpu_state
	mov eax, 0x7fffffff
	mov edx, 0x7fffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxsave
	xsave [rel temp_fpu_save]
	ret
.fxsave:
	fxsave [rel temp_fpu_save]
	ret

sym restore_temp_fpu_state
	mov eax, 0x7fffffff
	mov edx, 0x7fffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxrstor
	xrstor [rel temp_fpu_save]
	ret
.fxrstor:
	fxrstor [rel temp_fpu_save]
	ret

sym save_fpu_state
	mov eax, 0x7fffffff
	mov edx, 0x7fffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxsave
	xsave [rel fpu_save]
	ret
.fxsave:
	fxsave [rel fpu_save]
	ret

sym restore_fpu_state
	mov eax, 0x7fffffff
	mov edx, 0x7fffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxrstor
	xrstor [rel fpu_save]
	ret
.fxrstor:
	fxrstor [rel fpu_save]
	ret

; close your eyes
sym asm_resume
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
	mov ax, ds
	mov WORD [rel caller_ds_save], ax
	mov ax, ss
	mov WORD [rel caller_ss_save], ax
	mov ax, es
	mov WORD [rel caller_es_save], ax
	mov ax, fs
	mov WORD [rel caller_fs_save], ax
	mov ax, gs
	mov WORD [rel caller_gs_save], ax

	mov ax, [rel ds_save]
	mov ds, ax
	mov ax, [rel ss_save]
	mov ss, ax
	mov ax, [rel es_save]
	mov es, ax
	; /* not possible on newer kernels, removing this entirely allows crashes
	;    on older kernels, but that was never a big deal anyway */
	; mov ax, [rel fs_save]
	; mov fs, ax
	; mov ax, [rel gs_save]
	; mov gs, ax
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

sym asm_exit_context
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
	mov ax, ds
	mov WORD [rel ds_save], ax
	mov ax, ss
	mov WORD [rel ss_save], ax
	mov ax, es
	mov WORD [rel es_save], ax
	mov ax, fs
	mov WORD [rel fs_save], ax
	mov ax, gs
	mov WORD [rel gs_save], ax

sym reload_state
	mov ax, [rel caller_ds_save]
	mov ds, ax
	mov ax, [rel caller_ss_save]
	mov ss, ax
	mov ax, [rel caller_es_save]
	mov es, ax
	; /* not possible on newer kernels (see line 211) */
	; mov ax, [rel caller_fs_save]
	; mov fs, ax
	; mov ax, [rel caller_gs_save]
	; mov gs, ax
	call restore_caller_fpu_state
	mov rax, [rel caller_rflags_save]
	push rax
	popfq
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

sym safe_memcpy
	mov DWORD [rel temp_context_switching], 1
	mov QWORD [rel temp_rax_save], rax
	mov QWORD [rel temp_rbx_save], rbx
	mov QWORD [rel temp_rcx_save], rcx
	mov QWORD [rel temp_rdx_save], rdx
	mov QWORD [rel temp_rsp_save], rsp
	mov QWORD [rel temp_rbp_save], rbp
	mov QWORD [rel temp_rsi_save], rsi
	mov QWORD [rel temp_rdi_save], rdi
	mov QWORD [rel temp_r8_save], r8
	mov QWORD [rel temp_r9_save], r9
	mov QWORD [rel temp_r10_save], r10
	mov QWORD [rel temp_r11_save], r11
	mov QWORD [rel temp_r12_save], r12
	mov QWORD [rel temp_r13_save], r13
	mov QWORD [rel temp_r14_save], r14
	mov QWORD [rel temp_r15_save], r15

	pushfq
	pop rax
	mov QWORD [rel temp_rflags_save], rax

	call save_temp_fpu_state
	mov rax, QWORD [rel temp_rax_save]
	mov rdx, QWORD [rel temp_rdx_save]

	call [rel memcpy_address]

sym temp_reload_state
	mov rax, [rel temp_rflags_save]
	push rax
	popfq

	call restore_temp_fpu_state

	mov rax, [rel temp_rax_save]
	mov rbx, [rel temp_rbx_save]
	mov rcx, [rel temp_rcx_save]
	mov rdx, [rel temp_rdx_save]
	mov rsp, [rel temp_rsp_save]
	mov rbp, [rel temp_rbp_save]
	mov rsi, [rel temp_rsi_save]
	mov rdi, [rel temp_rdi_save]
	mov r8, [rel temp_r8_save]
	mov r9, [rel temp_r9_save]
	mov r10, [rel temp_r10_save]
	mov r11, [rel temp_r11_save]
	mov r12, [rel temp_r12_save]
	mov r13, [rel temp_r13_save]
	mov r14, [rel temp_r14_save]
	mov r15, [rel temp_r15_save]
	mov DWORD [rel temp_context_switching], 0
	ret

sym fpu_float_to_double
	fld TWORD [rdi]
	fstp QWORD [rel fpu_temp]
	movsd xmm0, [rel fpu_temp]
	ret

sym setup_cpu
	push ax
	mov ax, ds
	mov WORD [rel ds_save], ax
	mov ax, ss
	mov WORD [rel ss_save], ax
	mov ax, es
	mov WORD [rel es_save], ax
	mov ax, fs
	mov WORD [rel fs_save], ax
	mov ax, gs
	mov WORD [rel gs_save], ax
	pop ax
	ret

sym setup_fpu
	push rax
	push rbx
	push rcx
	push rdx

	mov DWORD [rel xsave_supported], 0
	mov DWORD [rel fxsave_supported], 1 ; YES

	mov eax, 0x01
	cpuid
	test ecx, 1 << 28
	jz .no_avx
	mov DWORD [rel xsave_supported], 1 ; YES
.no_avx:
	call save_caller_fpu_state
	call restore_fpu_state
	finit
	ldmxcsr [rel mxcsr_fixup]	; seriously? come on
	call save_fpu_state
	call restore_caller_fpu_state
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret
