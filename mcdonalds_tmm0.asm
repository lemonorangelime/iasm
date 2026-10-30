; iasm macro

extern tile0_save
extern reload_state
extern us_items
tmm0_override_exploit:
	mov rdi, tile0_save
	mov rsi, us_items
	mov rcx, 256
	xor rax, rax
.loop:
	mov edx, DWORD [rsi]
	mov DWORD [rdi], edx
	add rsi, 16
	add rdi, 4
	sub rcx, 1
	cmp rax, rcx
	jne .loop
~	add rsp, 8		; fix stack because we never return
~	jmp reload_state	; corrupt iasm internal state and allow tmm0 override

call tmm0_override_exploit
