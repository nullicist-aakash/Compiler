section .text
	default rel
	extern printf
	extern scanf
	global main


main:
	finit
	push rbp
	mov rbp, rsp
	sub rsp, 32

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
.label32:

	; Push b5
	mov ax, word [rbp - 8]
	push ax

	; Push d5cb34567
	mov ax, word [rbp - 10]
	push ax

	; if <=, JMP Label#33
	pop bx
	pop ax
	cmp ax, bx
	jle .label33
	jmp .label31
.label33:

	; Reading variable: b3c2.maths
	lea rsi, [rbp - 16]
	mov rdi, int_in
	xor rax, rax
	call scanf

	; Reading variable: b3c2.physics
	lea rsi, [rbp - 14]
	mov rdi, int_in
	xor rax, rax
	call scanf

	; Reading variable: b3c2.chemistry
	lea rsi, [rbp - 12]
	mov rdi, int_in
	xor rax, rax
	call scanf

	; Push b5
	mov ax, word [rbp - 8]
	push ax

	; Push 100
	mov ax, 100d
	push ax

	; add
	pop bx
	pop ax
	add ax, bx
	push ax

	; Assign to b3c34.studentid
	pop ax
	mov word [rbp - 26], ax

	; Push b3c2.physics
	mov ax, word [rbp - 14]
	push ax

	; Assign to b3c34.marks.physics
	pop ax
	mov word [rbp - 22], ax

	; Push b3c2.chemistry
	mov ax, word [rbp - 12]
	push ax

	; Assign to b3c34.marks.chemistry
	pop ax
	mov word [rbp - 20], ax

	; Push b3c2.maths
	mov ax, word [rbp - 16]
	push ax

	; Assign to b3c34.marks.maths
	pop ax
	mov word [rbp - 24], ax

	; Push b3c34.marks.maths
	mov ax, word [rbp - 24]
	push ax

	; Push b3c34.marks.physics
	mov ax, word [rbp - 22]
	push ax

	; if >=, JMP Label#42
	pop bx
	pop ax
	cmp ax, bx
	jge .label42
	jmp .label40
.label42:

	; Push b3c34.marks.chemistry
	mov ax, word [rbp - 20]
	push ax

	; Push b3c34.marks.maths
	mov ax, word [rbp - 24]
	push ax

	; if <=, JMP Label#41
	pop bx
	pop ax
	cmp ax, bx
	jle .label41
	jmp .label40
.label41:

	; Push 1
	mov ax, 1d
	push ax

	; Assign to b3c34.tagvalue
	pop ax
	mov word [rbp - 28], ax

	; Push b3c34.marks.maths
	mov ax, word [rbp - 24]
	push ax

	; Assign to b3c34.maximummarks.maths
	pop ax
	mov word [rbp - 18], ax
.label40:

	; Writing variable: b3c34.tagvalue
	mov ax, word [rbp - 28]
	movsx rsi, ax
	mov rdi, int_out
	xor rax, rax
	call printf

	; Writing variable: b3c34.studentid
	mov ax, word [rbp - 26]
	movsx rsi, ax
	mov rdi, int_out
	xor rax, rax
	call printf

	; Writing variable: b3c34.marks.maths
	mov ax, word [rbp - 24]
	movsx rsi, ax
	mov rdi, int_out
	xor rax, rax
	call printf

	; Writing variable: b3c34.marks.physics
	mov ax, word [rbp - 22]
	movsx rsi, ax
	mov rdi, int_out
	xor rax, rax
	call printf

	; Writing variable: b3c34.marks.chemistry
	mov ax, word [rbp - 20]
	movsx rsi, ax
	mov rdi, int_out
	xor rax, rax
	call printf

	; Writing variable: b3c34.maximummarks.chemistry
	mov ax, word [rbp - 18]
	movsx rsi, ax
	mov rdi, int_out
	xor rax, rax
	call printf

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
	jmp .label32
.label31:

.exit:
	add rsp, 32
	pop rbp
	mov rax, 0
	ret

section .data
	int_out:  db  "%hd", 10, 0
	int_in:  db  "%hd", 0
	real_out:  db  "%f", 10, 0
	real_in:  db  "%f", 0
	real_val:  db  0,0,0,0
