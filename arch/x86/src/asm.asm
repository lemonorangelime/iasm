bits 32

%macro sym 1-2
	extern %1
	%1: %2
%endmacro

extern memcpy

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
sym eax_save, resd 1
sym ebx_save, resd 1
sym ecx_save, resd 1
sym edx_save, resd 1
sym esp_save, resd 1
sym ebp_save, resd 1
sym esi_save, resd 1
sym edi_save, resd 1
sym eflags_save, resd 1

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

sym return_point, resd 1
sym context_switching, resd 1
sym temp_context_switching, resd 1

sym xsave_supported, resd 1
sym fxsave_supported, resd 1

section .text

sym save_caller_fpu_state
	mov eax, 0xffffffff
	mov edx, 0xffffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxsave
	xsave [rel caller_fpu_save]
	ret
.fxsave:
	fxsave [rel caller_fpu_save]
	ret

sym restore_caller_fpu_state
	mov eax, 0xffffffff
	mov edx, 0xffffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxrstor
	xrstor [rel caller_fpu_save]
	ret
.fxrstor:
	fxrstor [rel caller_fpu_save]
	ret

sym save_temp_fpu_state
	mov eax, 0xffffffff
	mov edx, 0xffffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxsave
	xsave [rel temp_fpu_save]
	ret
.fxsave:
	fxsave [rel temp_fpu_save]
	ret

sym restore_temp_fpu_state
	mov eax, 0xffffffff
	mov edx, 0xffffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxrstor
	xrstor [rel temp_fpu_save]
	ret
.fxrstor:
	fxrstor [rel temp_fpu_save]
	ret

sym save_fpu_state
	mov eax, 0xffffffff
	mov edx, 0xffffffff
	cmp DWORD [rel xsave_supported], 0
	je .fxsave
	xsave [rel fpu_save]
	ret
.fxsave:
	fxsave [rel fpu_save]
	ret

sym restore_fpu_state
	mov eax, 0xffffffff
	mov edx, 0xffffffff
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

sym asm_continue
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

sym reload_state
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
	;fst QWORD [rel fpu_temp]
	;movsd xmm0, [rel fpu_temp]
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
	call save_fpu_state
	call restore_caller_fpu_state
	pop edx
	pop ecx
	pop ebx
	pop eax
	ret
