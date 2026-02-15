bits 32

extern float32_decode
extern float64_decode

section .bss

align 64
temp: resq 0

section .text

float64_decode:
	fld QWORD [esp + 4]
	ret

float32_decode:
	fld DWORD [esp + 4]
	ret
