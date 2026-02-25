bits 32

%macro sym 1-2
	extern %1
	%1: %2
%endmacro

extern memcpy

section .data

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
sym eax_save, resd 1
sym ebx_save, resd 1
sym ecx_save, resd 1
sym edx_save, resd 1
sym esp_save, resd 1
sym ebp_save, resd 1
sym esi_save, resd 1
sym edi_save, resd 1
sym eflags_save, resd 1
sym return_point, resd 1
sym cs_save, resw 1
sym ds_save, resw 1
sym ss_save, resw 1
sym es_save, resw 1
sym fs_save, resw 1
sym gs_save, resw 1
sym register_save_end

sym caller_register_save
sym caller_eax_save, resd 1
sym caller_ebx_save, resd 1
sym caller_ecx_save, resd 1
sym caller_edx_save, resd 1
sym caller_esp_save, resd 1
sym caller_ebp_save, resd 1
sym caller_esi_save, resd 1
sym caller_edi_save, resd 1
sym caller_eflags_save, resd 1
sym caller_cs_save, resw 1
sym caller_ds_save, resw 1
sym caller_ss_save, resw 1
sym caller_es_save, resw 1
sym caller_fs_save, resw 1
sym caller_gs_save, resw 1

sym temp_register_save
sym temp_eax_save, resd 1
sym temp_ebx_save, resd 1
sym temp_ecx_save, resd 1
sym temp_edx_save, resd 1
sym temp_esp_save, resd 1
sym temp_ebp_save, resd 1
sym temp_esi_save, resd 1
sym temp_edi_save, resd 1
sym temp_eflags_save, resd 1

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
	mov DWORD [rel caller_eax_save], eax
	mov DWORD [rel caller_ebx_save], ebx
	mov DWORD [rel caller_ecx_save], ecx
	mov DWORD [rel caller_edx_save], edx
	mov DWORD [rel caller_esp_save], esp
	mov DWORD [rel caller_ebp_save], ebp
	mov DWORD [rel caller_esi_save], esi
	mov DWORD [rel caller_edi_save], edi
	pushfd
	pop eax
	mov DWORD [rel caller_eflags_save], eax
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
	mov ax, [rel fs_save]
	mov fs, ax
	mov ax, [rel gs_save]
	mov gs, ax
	call restore_fpu_state
	mov eax, [rel eflags_save]
	push eax
	popfd
	mov eax, [rel eax_save]
	mov ebx, [rel ebx_save]
	mov ecx, [rel ecx_save]
	mov edx, [rel edx_save]
	mov esp, [rel esp_save]
	mov ebp, [rel ebp_save]
	mov esi, [rel esi_save]
	mov edi, [rel edi_save]

	jmp [rel return_point]

sym asm_exit_context
	mov DWORD [rel eax_save], eax
	mov DWORD [rel ebx_save], ebx
	mov DWORD [rel ecx_save], ecx
	mov DWORD [rel edx_save], edx
	mov DWORD [rel esp_save], esp
	mov DWORD [rel ebp_save], ebp
	mov DWORD [rel esi_save], esi
	mov DWORD [rel edi_save], edi
	mov esp, [rel caller_esp_save]
	pushfd
	pop eax
	mov DWORD [rel eflags_save], eax
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
	mov ax, [rel caller_fs_save]
	mov fs, ax
	mov ax, [rel caller_gs_save]
	mov gs, ax
	mov eax, [rel caller_eflags_save]
	push eax
	popfd
	call restore_caller_fpu_state
	mov eax, [rel caller_eax_save]
	mov ebx, [rel caller_ebx_save]
	mov ecx, [rel caller_ecx_save]
	mov edx, [rel caller_edx_save]
	mov esp, [rel caller_esp_save]
	mov ebp, [rel caller_ebp_save]
	mov esi, [rel caller_esi_save]
	mov edi, [rel caller_edi_save]
	mov DWORD [rel context_switching], 0
	ret

sym safe_memcpy
	mov DWORD [rel temp_context_switching], 1
	mov DWORD [rel temp_eax_save], eax
	mov DWORD [rel temp_ebx_save], ebx
	mov DWORD [rel temp_ecx_save], ecx
	mov DWORD [rel temp_edx_save], edx
	mov DWORD [rel temp_esp_save], esp
	mov DWORD [rel temp_ebp_save], ebp
	mov DWORD [rel temp_esi_save], esi
	mov DWORD [rel temp_edi_save], edi

	pushfd
	pop eax
	mov DWORD [rel temp_eflags_save], eax

	call save_temp_fpu_state

	mov eax, [esp + 12]
	push eax
	mov eax, [esp + 12]
	push eax
	mov eax, [esp + 12]
	push eax

	mov eax, DWORD [rel temp_eax_save]
	mov edx, DWORD [rel temp_edx_save]

	call memcpy
	add esp, 12

sym temp_reload_state
	mov eax, [rel temp_eflags_save]
	push eax
	popfd

	call restore_temp_fpu_state

	mov eax, [rel temp_eax_save]
	mov ebx, [rel temp_ebx_save]
	mov ecx, [rel temp_ecx_save]
	mov edx, [rel temp_edx_save]
	mov esp, [rel temp_esp_save]
	mov ebp, [rel temp_ebp_save]
	mov esi, [rel temp_esi_save]
	mov edi, [rel temp_edi_save]
	mov DWORD [rel temp_context_switching], 0
	ret

sym fpu_float_to_double
	push eax
	mov eax, [esp + 4]
	fld TWORD [eax]
	pop eax
	ret

sym setup_cpu
	push eax
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

	mov eax, 0x202
	mov DWORD [rel eflags_save], eax

	pop eax
	ret

sym setup_fpu
	push eax
	push ebx
	push ecx
	push edx

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
	ldmxcsr [mxcsr_fixup]

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
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret
