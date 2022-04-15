#include "Assembly.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

ASTNode* func;
FuncEntry* entry;
Trie* localSymbolTable;
FILE* fp_output;

char* prefix = "global _start\n\nsection .text\n__int_to_str:\n	push rax\n	push rbx\n	push rcx\n	push rdx\n	\n	mov rax, 0\n	mov ax, [integer]\n	mov rbx, 0\n	mov cx, 0ah\n	mov rdx, 0\n	\n	cmp ax, 0\n	jge .label1\n	\n	; store minus\n	mov byte [buff + rbx], '-'\n	inc bx\n	\n	; multiply ax by -1\n	xor ax, 0FFFFh\n	add ax, 1\n	\n.label1:\n	div cx\n	\n	; dx <- remainder, ax <- ax / 10\n	add dx, 030h\n	mov byte [buff + rbx], dl\n	inc rbx\n\n	mov dx, 0\n	cmp ax, 0\n	jnz .label1\n	\n	mov byte [buff + rbx], 10\n	mov byte [buff + rbx + 1], 0\n	mov rdx, rbx\n	inc rdx\n	dec rbx\n	\n	; Now reverse the string\n	mov rax, 0\n	\n	cmp byte [buff], '-'\n	jnz .label2\n	mov rax, 1\n	\n.label2:\n	cmp rax, rbx\n	jge .label3\n	mov cl, [buff + rbx]\n	mov ch, [buff + rax]\n	mov byte [buff + rbx], ch\n	mov byte [buff + rax], cl\n	\n	inc rax\n	dec rbx\n	jmp .label2\n	\n.label3:\n	cmp rdx, buff_size\n	jz .label4\n	mov byte [buff + rdx], 0\n	inc rdx\n	jmp .label3\n\n.label4:\n	pop rdx\n	pop rcx\n	pop rbx\n	pop rax\n	ret\n\n__str_to_int:\n	push rax\n	push rbx\n	push rcx\n	push rdx\n	\n	mov ax, 0\n	mov cx, 0\n	mov cl, byte [buff]\n	mov rbx, 0\n	\n	cmp cl, '-'\n	jnz .label1\n	mov rbx, 1\n	\n.label1:\n	; move arr[rbx] to cl\n	mov cl, byte [buff + rbx]\n	inc rbx\n	\n	cmp cl, 10\n	je .label2\n	\n	; ax = ax * 10 + cl\n	mov dx, 10d\n	mul dx\n	sub cl, '0'\n	add ax, cx\n	\n	jmp .label1\n\n.label2:\n	mov cl, byte [buff]\n	cmp cl, '-'\n	jnz .label3\n	\n	xor ax, 0ffffh\n	add ax, 1\n	\n.label3:\n	mov word [integer], ax\n	\n	pop rdx\n	pop rcx\n	pop rbx\n	pop rax\n	ret\n\n";
char* postfix = "\nexit:\n    mov     rax, 60\n    mov     rdi, 0\n    syscall\n	\n	\nsection .data\ninteger      dw  0\nreal		 dq  0\nbuff        db 10 dup(0)\nbuff_size    equ $-buff\n";

int* calcByteAddress(char* name)
{
	int argc = 1;
	for (char* c = name; *c; c++)
		if (*c == '.')
			++argc;

	char* tmp = calloc(strlen(name) + 1, sizeof(char));
	strcpy(tmp, name);

	char** argv = calloc(argc, sizeof(char*));
	argv[0] = tmp;

	argc = 1;
	for (char* c = tmp; *c; c++)
		if (*c == '.')
			argv[argc++] = c + 1, *c = '\0';

	TypeLog* entry = trie_getRef(localSymbolTable, argv[0])->entry.ptr;

	if (entry == NULL)
		entry = trie_getRef(globalSymbolTable, argv[0])->entry.ptr;

	VariableEntry* varEntry = entry->structure;

	if (argc == 1)
	{
		free(tmp);
		free(argv);

		int* x = calloc(2, sizeof(int));
		x[0] = varEntry->offset + varEntry->type->width;
		x[1] = varEntry->type->width;
		return x;
	}

	assert(0);
	return NULL;
}

// Write globals
void fillDataSegment(char* key, TrieEntry* entry)
{
	TypeLog* typelog = entry->ptr;

	if (typelog->entryType != VARIABLE)
		return;

	VariableEntry* var = typelog->structure;

	fprintf(fp_output, "%s:\tdb\t%d\tdup(0)\n", var->name, var->type->width);
}

void readVariable(char* name)
{
	int* pair = calcByteAddress(name);
	int address = pair[0];
	int width = pair[1];
	free(pair);

	fprintf(fp_output, "\n\t; Reading variable: %s\n", name);
	
	if (width > 2)
		fprintf(fp_output, "\tcall exit\n");

	fprintf(fp_output, "\tmov rax, 0\n\tmov rdi, 0\n\tmov rsi, buff\n\tmov rdx, buff_size\n\tsyscall\n");
	fprintf(fp_output, "\tcall __str_to_int\n\tpush ax\n\tmov ax, [integer]\n");
	fprintf(fp_output, "\tmov word [rbp + %dd], ax\n\tpop ax\n", address);
}

void writeVariable(char* name)
{
	int* pair = calcByteAddress(name);
	int address = pair[0];
	int width = pair[1];
	free(pair);

	fprintf(fp_output, "\n\t; Writing variable: %s\n", name);

	if (width > 2)
		fprintf(fp_output, "\tcall exit\n");

	fprintf(fp_output, "\tpush ax\n\tmov ax, word [rbp + %dd]\n\tmov [integer], ax\n\tpop ax\n", address);
	fprintf(fp_output, "\tcall __int_to_str\n\tmov rax, 1\n\tmov rdi, 1\n\tmov rsi, buff\n\tmov rdx, buff_size\n\tsyscall\n");
}

void handleFunction(IRInsNode* funcCode)
{
	// assuming this is main for now
	fprintf(fp_output, "\tpush rbp\n");
	fprintf(fp_output, "\tmov rbp, rsp\n");
	fprintf(fp_output, "\tsub rsp, %d\n", entry->activationRecordSize);

	for (; funcCode; funcCode = funcCode->next)
	{
		IRInstr* instr = funcCode->ins;

		if (instr->op == OP_JMP)
			fprintf(fp_output, "\tjmp .label%d\n", instr->src1.label);
		else if (instr->op == OP_LABEL)
			fprintf(fp_output, ".label%d\n", instr->src1.label);
		else if (instr->op == OP_ASSIGN)
		{
			int* pair = calcByteAddress(instr->src1.name);
			int address = pair[0];
			int width = pair[1];
			free(pair);

			fprintf(fp_output, "\n\t; Assign to %s\n", instr->src1.name);

			if (width > 2)
				fprintf(fp_output, "\tcall exit\n");
		
			fprintf(fp_output, "\tpop ax\n");
			fprintf(fp_output, "\tmov word [rbp + %dd], ax\n", address);
		}
		else if (instr->op == OP_CALL)
			fprintf(fp_output, "\tcall exit\n");
		else if (instr->op == OP_RET)
			fprintf(fp_output, "\tret\n");
		else if (instr->op == OP_ADD || instr->op == OP_SUB)
		{
			char* ins = instr->op == OP_ADD ? "add" : "sub";
			fprintf(fp_output, "\n\t; %s\n", ins);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 > 2 || width2 > 2)
				fprintf(fp_output, "\tcall exit\n");

			fprintf(fp_output, "\tpop bx\n");
			fprintf(fp_output, "\tpop ax\n");
			fprintf(fp_output, "\t%s ax, bx\n", ins);
			fprintf(fp_output, "\tpush ax\n");
		}
		else if (instr->op == OP_MUL)
		{
			fprintf(fp_output, "\n\t; multiply\n");

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 > 2 || width2 > 2)
				fprintf(fp_output, "\tcall exit\n");

			fprintf(fp_output, "\tpop bx\n");
			fprintf(fp_output, "\tpop ax\n");
			fprintf(fp_output, "\tmul bx\n");
			fprintf(fp_output, "\tpush ax\n");
		}
		else if (instr->op == OP_DIV)
		{
			fprintf(fp_output, "\n\t; multiply\n");

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 > 2 || width2 > 2)
				fprintf(fp_output, "\tcall exit\n");

			fprintf(fp_output, "\tpop bx\n");
			fprintf(fp_output, "\tpop ax\n");
			fprintf(fp_output, "\tmov dx, 0\n");
			fprintf(fp_output, "\tdiv bx\n");
			fprintf(fp_output, "\tpush ax\n");
		}
		else if (instr->op == OP_LE)
		{
			fprintf(fp_output, "\n\t; if <=, JMP Label#%d\n", instr->dst.label);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 > 2 || width2 > 2)
				fprintf(fp_output, "\tcall exit\n");

			fprintf(fp_output, "\tpop bx\n");
			fprintf(fp_output, "\tpop ax\n");
			fprintf(fp_output, "\tcmp ax, bx\n");
			fprintf(fp_output, "\tjle .label%d\n", instr->dst.label);
		}
		else if (instr->op == OP_GE)
		{
			fprintf(fp_output, "\n\t; if >=, JMP Label#%d\n", instr->dst.label);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 > 2 || width2 > 2)
				fprintf(fp_output, "\tcall exit\n");

			fprintf(fp_output, "\tpop bx\n");
			fprintf(fp_output, "\tpop ax\n");
			fprintf(fp_output, "\tcmp ax, bx\n");
			fprintf(fp_output, "\tjge .label%d\n", instr->dst.label);
		}
		else if (instr->op == OP_LT)
		{
			fprintf(fp_output, "\n\t; if <, JMP Label#%d\n", instr->dst.label);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 > 2 || width2 > 2)
				fprintf(fp_output, "\tcall exit\n");

			fprintf(fp_output, "\tpop bx\n");
			fprintf(fp_output, "\tpop ax\n");
			fprintf(fp_output, "\tcmp ax, bx\n");
			fprintf(fp_output, "\tjl .label%d\n", instr->dst.label);
		}
		else if (instr->op == OP_GT)
		{
			fprintf(fp_output, "\n\t; if >, JMP Label#%d\n", instr->dst.label);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 > 2 || width2 > 2)
				fprintf(fp_output, "\tcall exit\n");

			fprintf(fp_output, "\tpop bx\n");
			fprintf(fp_output, "\tpop ax\n");
			fprintf(fp_output, "\tcmp ax, bx\n");
			fprintf(fp_output, "\tjg .label%d\n", instr->dst.label);
		}
		else if (instr->op == OP_EQ)
		{
			fprintf(fp_output, "\n\t; if ==, JMP Label#%d\n", instr->dst.label);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 > 2 || width2 > 2)
				fprintf(fp_output, "\tcall exit\n");

			fprintf(fp_output, "\tpop bx\n");
			fprintf(fp_output, "\tpop ax\n");
			fprintf(fp_output, "\tcmp ax, bx\n");
			fprintf(fp_output, "\tjz .label%d\n", instr->dst.label);
		}
		else if (instr->op == OP_NEQ)
		{
			fprintf(fp_output, "\n\t; if !=, JMP Label#%d\n", instr->dst.label);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 > 2 || width2 > 2)
				fprintf(fp_output, "\tcall exit\n");

			fprintf(fp_output, "\tpop bx\n");
			fprintf(fp_output, "\tpop ax\n");
			fprintf(fp_output, "\tcmp ax, bx\n");
			fprintf(fp_output, "\tjnz .label%d\n", instr->dst.label);
		}
		else if (instr->op == OP_READ)
			readVariable(instr->src1.name);
		else if (instr->op == OP_WRITE)
			writeVariable(instr->src1.name);
		else if (instr->op == OP_PUSH)
		{
			int* pair = calcByteAddress(instr->src1.name);
			int address = pair[0];
			int width = pair[1];
			free(pair);

			fprintf(fp_output, "\n\t; Push %s\n", instr->src1.name);

			if (width > 2)
				fprintf(fp_output, "\tcall exit\n");

			fprintf(fp_output, "\tmov ax, word [rbp + %dd\n", address);
			fprintf(fp_output, "\tpush ax\n");
		}
		else if (instr->op == OP_PUSHI)
		{
			fprintf(fp_output, "\n\t; Push %d\n", instr->src1.int_val);
			fprintf(fp_output, "\tmov ax, %dd\n", instr->src1.int_val);
			fprintf(fp_output, "\tpush ax\n");
		}
	}
}

void generateAssembly(FILE* fp, ASTNode* rt, IRInsNode** functions)
{
	ASTNode* root = rt;
	fp_output = fp;

	fprintf(fp, "%s", prefix);

	int i = 0;
	func = root->children[0];

	while (func)
	{
		TypeLog* mediator = trie_getRef(globalSymbolTable, func->token->lexeme)->entry.ptr;
		FuncEntry* entry = mediator->structure;
		localSymbolTable = entry->symbolTable;

		fprintf(fp, "%s:\n", func->token->lexeme);
		handleFunction(functions[i++]);
		func = func->sibling;
	}

	func = rt->children[1];
	TypeLog* mediator = trie_getRef(globalSymbolTable, func->token->lexeme)->entry.ptr;
	entry = mediator->structure;
	localSymbolTable = entry->symbolTable;

	fprintf(fp, "\n_start:\nmov rax, rsp\n	and rax, 0fh\n	cmp rax, 0\n	jz .startlabel\n	sub rsp, 8\n	\n.startlabel:\n");
	handleFunction(functions[i++]);

	fprintf(fp, "%s", postfix);
	iterateTrie(globalSymbolTable, fillDataSegment);
}