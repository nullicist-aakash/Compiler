	global main
	extern printf
	extern scanf
	extern atoi
	extern atof

section .text

_one:
main:
	mov rax, 60                 ; system call for exit
	xor rdi, rdi                ; exit code 0
	syscall                     ; invoke operating system to exit

section .data
c3bd:	db	2	dup(0)
c6:	db	2	dup(0)
d7:	db	4	dup(0)
