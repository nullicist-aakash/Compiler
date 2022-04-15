global _start

section .text
__int_to_str:
	push rax
	push rbx
	push rcx
	push rdx
	
	mov rax, 0
	mov ax, [integer]
	mov rbx, 0
	mov cx, 0ah
	mov rdx, 0
	
	cmp ax, 0
	jge .label1
	
	; store minus
	mov byte [buff + rbx], '-'
	inc bx
	
	; multiply ax by -1
	xor ax, 0FFFFh
	add ax, 1
	
.label1:
	div cx
	
	; dx <- remainder, ax <- ax / 10
	add dx, 030h
	mov byte [buff + rbx], dl
	inc rbx

	mov dx, 0
	cmp ax, 0
	jnz .label1
	
	mov byte [buff + rbx], 10
	mov byte [buff + rbx + 1], 0
	mov rdx, rbx
	inc rdx
	dec rbx
	
	; Now reverse the string
	mov rax, 0
	
	cmp byte [buff], '-'
	jnz .label2
	mov rax, 1
	
.label2:
	cmp rax, rbx
	jge .label3
	mov cl, [buff + rbx]
	mov ch, [buff + rax]
	mov byte [buff + rbx], ch
	mov byte [buff + rax], cl
	
	inc rax
	dec rbx
	jmp .label2
	
.label3:
	cmp rdx, buff_size
	jz .label4
	mov byte [buff + rdx], 0
	inc rdx
	jmp .label3

.label4:
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret

__str_to_int:
	push rax
	push rbx
	push rcx
	push rdx
	
	mov ax, 0
	mov cx, 0
	mov cl, byte [buff]
	mov rbx, 0
	
	cmp cl, '-'
	jnz .label1
	mov rbx, 1
	
.label1:
	; move arr[rbx] to cl
	mov cl, byte [buff + rbx]
	inc rbx
	
	cmp cl, 10
	je .label2
	
	; ax = ax * 10 + cl
	mov dx, 10d
	mul dx
	sub cl, '0'
	add ax, cx
	
	jmp .label1

.label2:
	mov cl, byte [buff]
	cmp cl, '-'
	jnz .label3
	
	xor ax, 0ffffh
	add ax, 1
	
.label3:
	mov word [integer], ax
	
	pop rdx
	pop rcx
	pop rbx
	pop rax
	ret


_start:
mov rax, rsp
	and rax, 0fh
	cmp rax, 0
	jz .startlabel
	sub rsp, 8
	
.startlabel:
	push rbp
	mov rbp, rsp
	sub rsp, 8

	; Reading variable: b2
	mov rax, 0
	mov rdi, 0
	mov rsi, buff
	mov rdx, buff_size
	syscall
	call __str_to_int
	push ax
	mov ax, [integer]
	mov word [rbp + 4d], ax
	pop ax

	; Assign to c2
	pop ax
	mov word [rbp + 6d], ax

	; Reading variable: d2
	mov rax, 0
	mov rdi, 0
	mov rsi, buff
	mov rdx, buff_size
	syscall
	call __str_to_int
	push ax
	mov ax, [integer]
	mov word [rbp + 8d], ax
	pop ax

	; Push b2
	mov ax, word [rbp + 4d], ax
	push ax

	; Push c2
	mov ax, word [rbp + 6d], ax
	push ax

	; add
	pop bx
	pop ax
	add ax, bx
	push ax

	; Push d2
	mov ax, word [rbp + 8d], ax
	push ax

	; add
	pop bx
	pop ax
	add ax, bx
	push ax

	; Assign to b3
	pop ax
	mov word [rbp + 2d], ax

	; Writing variable: b2
	push ax
	mov ax, word [rbp + 4d]
	mov [integer], ax
	pop ax
	call __int_to_str
	mov rax, 1
	mov rdi, 1
	mov rsi, buff
	mov rdx, buff_size
	syscall
	ret

exit:
    mov     rax, 60
    mov     rdi, 0
    syscall
	
	
section .data
integer      dw  0
real		 dq  0
buff        db 10 dup(0)
buff_size    equ $-buff
