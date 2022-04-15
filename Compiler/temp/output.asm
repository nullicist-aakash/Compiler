section .text
	default rel
	extern printf
	extern scanf
	global main


main:
	push rbp
	mov rbp, rsp
	sub rsp, 16

	; Reading variable: b2
	lea rsi, [rbp - 4d]
	mov rdi, int_in
	xor rax, rax
	call scanf

	; Push 35
	mov ax, 35d
	push ax

	; Assign to c2
	pop ax
	mov word [rbp - 6d], ax

	; Reading variable: d2
	lea rsi, [rbp - 8d]
	mov rdi, int_in
	xor rax, rax
	call scanf

	; Push b2
	mov ax, word [rbp - 4d]
	push ax

	; Push c2
	mov ax, word [rbp - 6d]
	push ax

	; add
	pop bx
	pop ax
	add ax, bx
	push ax

	; Push d2
	mov ax, word [rbp - 8d]
	push ax

	; add
	pop bx
	pop ax
	add ax, bx
	push ax

	; Assign to b3
	pop ax
	mov word [rbp - 2d], ax

	; Writing variable: b3
	mov ax, word [rbp - 2d]
	movsx rsi, ax
	mov rdi, int_out
	xor rax, rax
	call printf
	add rsp, 16

	pop rbp
	mov rax, 0
	ret

section .data
	int_out:  db  "%hd", 10, 0
	int_in:  db  "%hd", 0
	real_out:  db  "%f", 10, 0
	real_in:  db  "%f", 0
