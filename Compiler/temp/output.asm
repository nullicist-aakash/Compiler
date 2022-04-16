section .text
	default rel
	extern printf
	extern scanf
	global main


main:
	finit
	push rbp
	mov rbp, rsp
	sub rsp, 16

	; Push 1
	mov ax, 1d
	push ax

	; Assign to b5
	pop ax
	mov word [rbp - 8], ax

	; Reading variable: d5cb34567
	lea rsi, [rbp - 10]
	mov rdi, int_in
	xor rax, rax
	call scanf

	; Push 0
	mov ax, 0d
	push ax

	; Assign to d4.maths
	pop ax
	mov word [rbp - 6], ax

	; Push 0
	mov ax, 0d
	push ax

	; Assign to d4.physics
	pop ax
	mov word [rbp - 4], ax

	; Push 0
	mov ax, 0d
	push ax

	; Assign to d4.chemistry
	pop ax
	mov word [rbp - 2], ax
.label20:

	; Push b5
	mov ax, word [rbp - 8]
	push ax

	; Push d5cb34567
	mov ax, word [rbp - 10]
	push ax

	; if <=, JMP Label#21
	pop bx
	pop ax
	cmp ax, bx
	jle .label21
	jmp .label19
.label21:

	; Reading variable: b3c2
	lea rsi, [rbp - 16]
	mov rdi, int_in
	xor rax, rax
	call scanf
	lea rsi, [rbp - 14]
	mov rdi, int_in
	xor rax, rax
	call scanf
	lea rsi, [rbp - 12]
	mov rdi, int_in
	xor rax, rax
	call scanf

	; Push b3c2.maths
	mov ax, word [rbp - 16]
	push ax

	; Push d4.maths
	mov ax, word [rbp - 6]
	push ax

	; add
	pop bx
	pop ax
	add ax, bx
	push ax

	; Assign to d4.maths
	pop ax
	mov word [rbp - 6], ax

	; Push b3c2.physics
	mov ax, word [rbp - 14]
	push ax

	; Push d4.physics
	mov ax, word [rbp - 4]
	push ax

	; add
	pop bx
	pop ax
	add ax, bx
	push ax

	; Assign to d4.physics
	pop ax
	mov word [rbp - 4], ax

	; Push b3c2.chemistry
	mov ax, word [rbp - 12]
	push ax

	; Push d4.chemistry
	mov ax, word [rbp - 2]
	push ax

	; add
	pop bx
	pop ax
	add ax, bx
	push ax

	; Assign to d4.chemistry
	pop ax
	mov word [rbp - 2], ax

	; Push b5
	mov ax, word [rbp - 8]
	push ax

	; Push 1
	mov ax, 1d
	push ax

	; add
	pop bx
	pop ax
	add ax, bx
	push ax

	; Assign to b5
	pop ax
	mov word [rbp - 8], ax
	jmp .label20
.label19:

	; Writing variable: d4
	mov ax, word [rbp - 6]
	movsx rsi, ax
	mov rdi, int_out
	xor rax, rax
	call printf
	mov ax, word [rbp - 4]
	movsx rsi, ax
	mov rdi, int_out
	xor rax, rax
	call printf
	mov ax, word [rbp - 2]
	movsx rsi, ax
	mov rdi, int_out
	xor rax, rax
	call printf

.exit:
	add rsp, 16
	pop rbp
	mov rax, 0
	ret

section .data
	int_out:  db  "%hd", 10, 0
	int_in:  db  "%hd", 0
	real_out:  db  "%f", 10, 0
	real_in:  db  "%f", 0
	real_val:  db  0,0,0,0
