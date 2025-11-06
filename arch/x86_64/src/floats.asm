bits 64

extern float32_decode
extern float64_decode

section .bss

align 64
temp: resq 0

section .text

float64_decode:
	lea rax, [rel temp]
	mov [rax], rdi
	movsd xmm0, [rax]
	ret

float32_decode:
	call float64_decode
	cvtss2sd xmm0, xmm0
	ret
