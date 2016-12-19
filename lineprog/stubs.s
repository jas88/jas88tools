[bits 64]

section .text
extern main
global _start
global writev
global read
global exit

_start:                ; ELF entry point
xor	ebp,ebp
mov	r9,rdx	; save exit point
pop	rdi
mov	rsi,rsp
call	main
mov	rax,60	; sys_exit
mov	rdi,0		; return value
syscall

writev:
mov	rax,20	; sys_writev
syscall
ret

read:
mov	rax,0	; sys_read
syscall
ret

exit:
mov	rax,60
syscall