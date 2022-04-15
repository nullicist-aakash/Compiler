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

	; Reading variable: b3c45
	lea rsi, [rbp - 2]
	mov rdi, int_in
	xor rax, rax
	call scanf

	; Reading variable: b2d6
	lea rsi, [rbp - 4]
	mov rdi, int_in
	xor rax, rax
	call scanf

	; Push 2097
	mov ax, 2097d
	push ax

	; Assign to b2bb3
	pop ax
	mov word [rbp - 6], ax

	; Push 66987
	mov ax, 6387d
	push ax

	; Assign to d6
	pop ax
	mov word [d6 - 0], ax

	; Push b3c45
	mov ax, word [rbp - 2]
	push ax

	; Push b2d6
	mov ax, word [rbp - 4]
	push ax

	; if <=, JMP Label#17
	pop bx
	pop ax
	cmp ax, bx
	jle .label17
	jmp .label16
.label17:

	; Push b2d6
	mov ax, word [rbp - 4]
	push ax

	; Push b2bb3
	mov ax, word [rbp - 6]
	push ax

	; if <=, JMP Label#15
	pop bx
	pop ax
	cmp ax, bx
	jle .label15
	jmp .label16
.label15:

	; Push d6
	mov ax, word [d6 - 0]
	push ax

	; Push 89
	mov ax, 89d
	push ax

	; sub
	pop bx
	pop ax
	sub ax, bx
	push ax

	; Push b2bb3
	mov ax, word [rbp - 6]
	push ax

	; add
	pop bx
	pop ax
	add ax, bx
	push ax

	; Assign to d6
	pop ax
	mov word [d6 - 0], ax
	jmp .label14
.label16:

	; Push d6
	mov ax, word [d6 - 0]
	push ax

	; Push b2bb3
	mov ax, word [rbp - 6]
	push ax

	; Push 3
	mov ax, 3d
	push ax

	; multiply
	pop bx
	pop ax
	mul bx
	push ax

	; sub
	pop bx
	pop ax
	sub ax, bx
	push ax

	; Assign to d6
	pop ax
	mov word [d6 - 0], ax
.label14:

	; Writing variable: d6
	mov ax, word [d6 - 0]
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
d6:	db	2	dup(0)
