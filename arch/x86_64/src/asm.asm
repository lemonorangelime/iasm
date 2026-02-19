
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
align 32
sym tile_cfg, times 512 db 0
align 64
sym avx512_ones
dq 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff
align 128
sym tile_save
sym tile0_save, times 1024 db 0
align 128
sym tile1_save, times 1024 db 0
align 128
sym tile2_save, times 1024 db 0
align 128
sym tile3_save, times 1024 db 0
align 128
sym tile4_save, times 1024 db 0
align 128
sym tile5_save, times 1024 db 0
align 128
sym tile6_save, times 1024 db 0
align 128
sym tile7_save, times 1024 db 0

section .bss

align 64
sym fpu_save, resb 16384
align 64
sym caller_fpu_save, resb 16384
align 64
sym temp_fpu_save, resb 16384
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
sym r16_save, resq 1
sym r17_save, resq 1
sym r18_save, resq 1
sym r19_save, resq 1
sym r20_save, resq 1
sym r21_save, resq 1
sym r22_save, resq 1
sym r23_save, resq 1
sym r24_save, resq 1
sym r25_save, resq 1
sym r26_save, resq 1
sym r27_save, resq 1
sym r28_save, resq 1
sym r29_save, resq 1
sym r30_save, resq 1
sym r31_save, resq 1
sym rflags_save, resq 1
sym return_point, resq 1
sym cs_save, resw 1
sym ds_save, resw 1
sym ss_save, resw 1
sym es_save, resw 1
sym fs_save, resw 1
sym gs_save, resw 1
sym register_save_end

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
sym tile_supported, resd 1
sym apx_supported, resd 1

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
	cmp DWORD [rel tile_supported], 0
	je .no_tile
	sttilecfg [rel tile_cfg]
	lea rax, [rel tile0_save]
	tilestored [rax], tmm0
	lea rax, [rel tile1_save]
	tilestored [rax], tmm1
	lea rax, [rel tile2_save]
	tilestored [rax], tmm2
	lea rax, [rel tile3_save]
	tilestored [rax], tmm3
	lea rax, [rel tile4_save]
	tilestored [rax], tmm4
	lea rax, [rel tile5_save]
	tilestored [rax], tmm5
	lea rax, [rel tile6_save]
	tilestored [rax], tmm6
	lea rax, [rel tile7_save]
	tilestored [rax], tmm7
.no_tile:
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
	jmp .continue
.fxrstor:
	fxrstor [rel fpu_save]
.continue:
	cmp DWORD [rel tile_supported], 0
	je .no_tile
	ldtilecfg [rel tile_cfg]
	lea rax, [rel tile0_save]
	tileloadd tmm0, [rax]
	lea rax, [rel tile1_save]
	tileloadd tmm1, [rax]
	lea rax, [rel tile2_save]
	tileloadd tmm2, [rax]
	lea rax, [rel tile3_save]
	tileloadd tmm3, [rax]
	lea rax, [rel tile4_save]
	tileloadd tmm4, [rax]
	lea rax, [rel tile5_save]
	tileloadd tmm5, [rax]
	lea rax, [rel tile6_save]
	tileloadd tmm6, [rax]
	lea rax, [rel tile7_save]
	tileloadd tmm7, [rax]
.no_tile:
	ret

sym save_apx_state
	cmp DWORD [rel apx_supported], 0
	je .no_apx
	mov QWORD [rel r16_save], r16
	mov QWORD [rel r17_save], r17
	mov QWORD [rel r18_save], r18
	mov QWORD [rel r19_save], r19
	mov QWORD [rel r20_save], r20
	mov QWORD [rel r21_save], r21
	mov QWORD [rel r22_save], r22
	mov QWORD [rel r23_save], r23
	mov QWORD [rel r24_save], r24
	mov QWORD [rel r25_save], r25
	mov QWORD [rel r26_save], r26
	mov QWORD [rel r27_save], r27
	mov QWORD [rel r28_save], r28
	mov QWORD [rel r29_save], r29
	mov QWORD [rel r30_save], r30
	mov QWORD [rel r31_save], r31
.no_apx:
	ret

sym restore_apx_state
	cmp DWORD [rel apx_supported], 0
	je .no_apx
	mov r16, [rel r16_save]
	mov r17, [rel r17_save]
	mov r18, [rel r18_save]
	mov r19, [rel r19_save]
	mov r20, [rel r20_save]
	mov r21, [rel r21_save]
	mov r22, [rel r22_save]
	mov r23, [rel r23_save]
	mov r24, [rel r24_save]
	mov r25, [rel r25_save]
	mov r26, [rel r26_save]
	mov r27, [rel r27_save]
	mov r28, [rel r28_save]
	mov r29, [rel r29_save]
	mov r30, [rel r30_save]
	mov r31, [rel r31_save]
.no_apx:
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

	call restore_apx_state
	mov ax, [rel ds_save]
	mov ds, ax
	mov ax, [rel ss_save]
	mov ss, ax
	mov ax, [rel es_save]
	mov es, ax
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
	call save_apx_state
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
	push rax
	push rbx
	push rcx
	push rdx
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

	mov eax, 7
	mov ecx, 1
	cpuid
	test edx, 1 << 21
	jz .no_apx
	mov DWORD [rel apx_supported], 1
.no_apx:
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret

sym setup_fpu
	push rax
	push rbx
	push rcx
	push rdx

	mov DWORD [rel xsave_supported], 0
	mov DWORD [rel tile_supported], 0
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

	mov eax, 7
	xor ecx, ecx
	cpuid
	test ebx, 1 << 16
	jz .noavx512

	vmovupd zmm0, [rel avx512_ones]
	vpmovq2m k0, zmm0
	vpmovq2m k1, zmm0
	vpmovq2m k2, zmm0
	vpmovq2m k3, zmm0
	vpmovq2m k3, zmm0
	vpmovq2m k4, zmm0
	vpmovq2m k5, zmm0
	vpmovq2m k6, zmm0
	vpmovq2m k7, zmm0
	vzeroall
.noavx512:

	call save_fpu_state
	call restore_caller_fpu_state



	xor rcx, rcx
	mov rax, 0x07
	cpuid
	test edx, 1 << 24
	jz .no_tile

	mov BYTE [rel tile_cfg], 1
	mov WORD [rel tile_cfg + 0x10], 0x40
	mov WORD [rel tile_cfg + 0x12], 0x40
	mov WORD [rel tile_cfg + 0x14], 0x40
	mov WORD [rel tile_cfg + 0x16], 0x40
	mov WORD [rel tile_cfg + 0x18], 0x40
	mov WORD [rel tile_cfg + 0x1a], 0x40
	mov WORD [rel tile_cfg + 0x1c], 0x40
	mov WORD [rel tile_cfg + 0x1e], 0x40
	mov rax, 0x1010101010101010
	mov QWORD [rel tile_cfg + 0x30], rax
	mov DWORD [rel tile_supported], 1
	ldtilecfg [rel tile_cfg]
.no_tile:



	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret
