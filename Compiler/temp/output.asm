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

	; Reading variable: b2
	lea rsi, [rbp - 8d]
	mov rdi, real_in
	xor rax, rax
	call scanf

	; Push 35.450001
	mov eax, __?float32?__(35.450001)
	mov dword [real_val], eax
	fld dword [real_val]

	; Assign to c2
	fstp dword [rbp - 12d]

	; Reading variable: d2
	lea rsi, [rbp - 16d]
	mov rdi, real_in
	xor rax, rax
	call scanf

	; Push b2
	fld dword [rbp - 8d]

	; Push c2
	fld dword [rbp - 12d]

	; add
	fadd

	; Push d2
	fld dword [rbp - 16d]

	; add
	fadd

	; Assign to b3
	fstp dword [rbp - 4d]

	; Writing variable: b3
	cvtss2sd xmm0, [real_val]
	mov      rdi, real_out            ; 1st arg to printf
	mov      rax, 1                 ; printf is varargs, there is 1 non-int argument

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
	real_val:  db  0cdh,0cch,019h,042h
	count:  dq       30
	sum:    dq       3
