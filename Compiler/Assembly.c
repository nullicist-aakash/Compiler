#include "Assembly.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

ASTNode* func;
FuncEntry* entry;
Trie* localSymbolTable;
FILE* fp_output;

char* prefix = "section .text\n\tdefault rel\n\textern printf\n\textern scanf\n\tglobal main\n\n";

VariableEntry* getVarEntry(char* name)
{
	TypeLog* entry = trie_getRef(localSymbolTable, name)->entry.ptr;

	if (entry == NULL)
		entry = trie_getRef(globalSymbolTable, name)->entry.ptr;

	return entry->structure;
}

int calcWidth(char* name)
{
	return getVarEntry(name)->type->width;
}

int getOffset(char* name)
{
	TypeLog* entry = trie_getRef(localSymbolTable, name)->entry.ptr;

	if (entry)
	{
		VariableEntry* var = entry->structure;
		return var->offset + var->type->width;
	}

	return 0;
}

char* getReferenceName(char* name)
{
	TypeLog* entry = trie_getRef(localSymbolTable, name)->entry.ptr;

	char buff[50];

	if (entry != NULL)
		strcpy(buff, "rbp");
	else
		strcpy(buff, name);

	entry = trie_getRef(globalSymbolTable, name)->entry.ptr;

	char* ret = calloc(50, sizeof(char));
	sprintf(ret, "[%s - %d]", buff, getOffset(name));
	return ret;
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
	fprintf(fp_output, "\n\t; Reading variable: %s\n", name);

	char* loc = getReferenceName(name);
	if (getVarEntry(name)->type->entryType == INT)
	{
		fprintf(fp_output, "\tlea rsi, %s\n", loc);
		fprintf(fp_output, "\tmov rdi, int_in\n");
		fprintf(fp_output, "\txor rax, rax\n");
		fprintf(fp_output, "\tcall scanf\n");
	}
	else
	{
		fprintf(fp_output, "\tlea rsi, %s\n", loc);
		fprintf(fp_output, "\tmov rdi, real_in\n");
		fprintf(fp_output, "\txor rax, rax\n");
		fprintf(fp_output, "\tcall scanf\n");
	}
	free(loc);
}

void writeVariable(char* name)
{
	fprintf(fp_output, "\n\t; Writing variable: %s\n", name);

	char* loc = getReferenceName(name);
	if (getVarEntry(name)->type->entryType == INT)
	{
		fprintf(fp_output, "\tmov ax, word %s\n", loc);
		fprintf(fp_output, "\tmovsx rsi, ax\n");
		fprintf(fp_output, "\tmov rdi, int_out\n");
		fprintf(fp_output, "\txor rax, rax\n");
		fprintf(fp_output, "\tcall printf\n");
	}
	else
	{
		fprintf(fp_output, "\tcvtss2sd xmm0, %s\n", loc);
		fprintf(fp_output, "\tmov rdi, real_out\n");
		fprintf(fp_output, "\tmov rax, 1\n");
		fprintf(fp_output, "\tcall printf\n");
	}
	free(loc);
}

void handleFunction(IRInsNode* funcCode)
{
	// assuming this is main for now
	fprintf(fp_output, "\tpush rbp\n");
	fprintf(fp_output, "\tmov rbp, rsp\n");

	int localSize = entry->activationRecordSize;
	localSize += (16 - (localSize % 16)) % 16;
	fprintf(fp_output, "\tsub rsp, %d\n", localSize);

	for (; funcCode; funcCode = funcCode->next)
	{
		IRInstr* instr = funcCode->ins;

		if (instr->op == OP_JMP)
			fprintf(fp_output, "\tjmp .label%d\n", instr->src1.label);
		else if (instr->op == OP_LABEL)
			fprintf(fp_output, ".label%d:\n", instr->src1.label);
		else if (instr->op == OP_ASSIGN)
		{
			fprintf(fp_output, "\n\t; Assign to %s\n", instr->src1.name);

			char* loc = getReferenceName(instr->src1.name);

			if (getVarEntry(instr->src1.name)->type->entryType == INT)
			{
				fprintf(fp_output, "\tpop ax\n");
				fprintf(fp_output, "\tmov word %s, ax\n", loc);
			}
			else
				fprintf(fp_output, "\tfstp dword %s\n", loc);

			free(loc);
		}
		else if (instr->op == OP_CALL)
			fprintf(fp_output, "\tjmp .exit\n");
		else if (instr->op == OP_ADD || instr->op == OP_SUB)
		{
			char* ins = instr->op == OP_ADD ? "add" : "sub";
			fprintf(fp_output, "\n\t; %s\n", ins);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 == 2 && width2 == 2)
			{
				fprintf(fp_output, "\tpop bx\n");
				fprintf(fp_output, "\tpop ax\n");
				fprintf(fp_output, "\t%s ax, bx\n", ins);
				fprintf(fp_output, "\tpush ax\n");
			}
			else
			{
				fprintf(fp_output, "\tf%s\n", ins);
			}
		}
		else if (instr->op == OP_MUL)
		{
			fprintf(fp_output, "\n\t; multiply\n");

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 == 2 && width2 == 2)
			{
				fprintf(fp_output, "\tpop bx\n");
				fprintf(fp_output, "\tpop ax\n");
				fprintf(fp_output, "\tmul bx\n");
				fprintf(fp_output, "\tpush ax\n");
			}
			else
			{
				fprintf(fp_output, "\tfmul\n");
			}
		}
		else if (instr->op == OP_DIV)
		{
			fprintf(fp_output, "\n\t; multiply\n");

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 == 2 && width2 == 2)
			{
				fprintf(fp_output, "\tpop bx\n");
				fprintf(fp_output, "\tpop ax\n");
				fprintf(fp_output, "\tmov dx, 0\n");
				fprintf(fp_output, "\tdiv bx\n");
				fprintf(fp_output, "\tpush ax\n");
			}
			else
				fprintf(fp_output, "\tfdiv\n");

		}
		else if (instr->op == OP_LE)
		{
			fprintf(fp_output, "\n\t; if <=, JMP Label#%d\n", instr->dst.label);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 == 2 && width2 == 2)
			{
				fprintf(fp_output, "\tpop bx\n");
				fprintf(fp_output, "\tpop ax\n");
				fprintf(fp_output, "\tcmp ax, bx\n");
				fprintf(fp_output, "\tjle .label%d\n", instr->dst.label);
			}
			else
			{
				fprintf(fp_output, "\tfcompp\n");
				fprintf(fp_output, "\tfstsw ax                      ;ax := fpu status register\n");
				fprintf(fp_output, "\tand eax, 0100011100000000B    ;take only condition code flags\n");
				fprintf(fp_output, "\tcmp    eax, 0000000100000000B ; is st0 < source ?\n");
				fprintf(fp_output, "\tje .label%d\n", instr->dst.label);
				fprintf(fp_output, "\tcmp    eax, 0100000000000000B ; is st0 = source ?\n");
				fprintf(fp_output, "\tje .label%d\n", instr->dst.label);
			}
		}
		else if (instr->op == OP_GE)
		{
			fprintf(fp_output, "\n\t; if >=, JMP Label#%d\n", instr->dst.label);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 == 2 && width2 == 2)
			{
				fprintf(fp_output, "\tpop bx\n");
				fprintf(fp_output, "\tpop ax\n");
				fprintf(fp_output, "\tcmp ax, bx\n");
				fprintf(fp_output, "\tjge .label%d\n", instr->dst.label);
			}
			else
			{
				fprintf(fp_output, "\tfcompp\n");
				fprintf(fp_output, "\tfstsw ax                      ;ax := fpu status register\n");
				fprintf(fp_output, "\tand eax, 0100011100000000B    ;take only condition code flags\n");
				fprintf(fp_output, "\tcmp    eax, 0000000000000000B ; is st0 > source ?\n");
				fprintf(fp_output, "\tje .label%d\n", instr->dst.label);
				fprintf(fp_output, "\tcmp    eax, 0100000000000000B ; is st0 = source ?\n");
				fprintf(fp_output, "\tje .label%d\n", instr->dst.label);
			}
		}
		else if (instr->op == OP_LT)
		{
			fprintf(fp_output, "\n\t; if <, JMP Label#%d\n", instr->dst.label);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 == 2 && width2 == 2)
			{
				fprintf(fp_output, "\tpop bx\n");
				fprintf(fp_output, "\tpop ax\n");
				fprintf(fp_output, "\tcmp ax, bx\n");
				fprintf(fp_output, "\tjl .label%d\n", instr->dst.label);
			}
			else
			{
				fprintf(fp_output, "\tfcompp\n");
				fprintf(fp_output, "\tfstsw ax                      ;ax := fpu status register\n");
				fprintf(fp_output, "\tand eax, 0100011100000000B    ;take only condition code flags\n");
				fprintf(fp_output, "\tcmp    eax, 0000000100000000B ; is st0 < source ?\n");
				fprintf(fp_output, "\tje .label%d\n", instr->dst.label);
			}
		}
		else if (instr->op == OP_GT)
		{
			fprintf(fp_output, "\n\t; if >, JMP Label#%d\n", instr->dst.label);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 == 2 && width2 == 2)
			{
				fprintf(fp_output, "\tpop bx\n");
				fprintf(fp_output, "\tpop ax\n");
				fprintf(fp_output, "\tcmp ax, bx\n");
				fprintf(fp_output, "\tjg .label%d\n", instr->dst.label);
			}
			else
			{
				fprintf(fp_output, "\tfcompp\n");
				fprintf(fp_output, "\tfstsw ax                      ;ax := fpu status register\n");
				fprintf(fp_output, "\tand eax, 0100011100000000B    ;take only condition code flags\n");
				fprintf(fp_output, "\tcmp    eax, 0000000000000000B ; is st0 > source ?\n");
				fprintf(fp_output, "\tje .label%d\n", instr->dst.label);
			}
		}
		else if (instr->op == OP_EQ)
		{
			fprintf(fp_output, "\n\t; if ==, JMP Label#%d\n", instr->dst.label);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 == 2 && width2 == 2)
			{
				fprintf(fp_output, "\tpop bx\n");
				fprintf(fp_output, "\tpop ax\n");
				fprintf(fp_output, "\tcmp ax, bx\n");
				fprintf(fp_output, "\tjz .label%d\n", instr->dst.label);
			}
			else
			{
				fprintf(fp_output, "\tfcompp\n");
				fprintf(fp_output, "\tfstsw ax                      ;ax := fpu status register\n");
				fprintf(fp_output, "\tand eax, 0100011100000000B    ;take only condition code flags\n");
				fprintf(fp_output, "\tcmp    eax, 0100000000000000B ; is st0 = source ?\n");
				fprintf(fp_output, "\tje .label%d\n", instr->dst.label);
			}
		}
		else if (instr->op == OP_NEQ)
		{
			fprintf(fp_output, "\n\t; if !=, JMP Label#%d\n", instr->dst.label);

			int width1 = instr->src1.type->width;
			int width2 = instr->src2.type->width;

			if (width1 == 2 && width2 == 2)
			{
				fprintf(fp_output, "\tpop bx\n");
				fprintf(fp_output, "\tpop ax\n");
				fprintf(fp_output, "\tcmp ax, bx\n");
				fprintf(fp_output, "\tjnz .label%d\n", instr->dst.label);
			}
			else
			{
				fprintf(fp_output, "\tfcompp\n");
				fprintf(fp_output, "\tfstsw ax                      ;ax := fpu status register\n");
				fprintf(fp_output, "\tand eax, 0100011100000000B    ;take only condition code flags\n");
				fprintf(fp_output, "\tcmp    eax, 0100000000000000B ; is st0 = source ?\n");
				fprintf(fp_output, "\tjne .label%d\n", instr->dst.label);
			}
		}
		else if (instr->op == OP_READ)
			readVariable(instr->src1.name);
		else if (instr->op == OP_WRITE)
			writeVariable(instr->src1.name);
		else if (instr->op == OP_PUSH)
		{
			fprintf(fp_output, "\n\t; Push %s\n", instr->src1.name);

			char* loc = getReferenceName(instr->src1.name);
			if (getVarEntry(instr->src1.name)->type->entryType == INT)
			{
				fprintf(fp_output, "\tmov ax, word %s\n", loc);
				fprintf(fp_output, "\tpush ax\n");
			}
			else
			{
				fprintf(fp_output, "\tfld dword %s\n", loc);
			}
			free(loc);
		}
		else if (instr->op == OP_PUSHI)
		{
			fprintf(fp_output, "\n\t; Push %d\n", instr->src1.int_val);
			fprintf(fp_output, "\tmov ax, %dd\n", instr->src1.int_val);
			fprintf(fp_output, "\tpush ax\n");
		}
		else if (instr->op == OP_PUSHR)
		{
			fprintf(fp_output, "\n\t; Push %f\n", instr->src1.real_val);
			fprintf(fp_output, "\tmov eax, __?float32?__(%f)\n", instr->src1.real_val);
			fprintf(fp_output, "\tmov dword [real_val], eax\n");
			fprintf(fp_output, "\tfld dword [real_val]\n");
		}
		else if (instr->op == OP_POP)
		{
			fprintf(fp_output, "\n\t; Pop\n");
			int width = instr->src1.type->width;

			if (instr->src1.type->entryType == REAL)
			{
				fprintf(fp_output, "\n\t; popping from floating point stack\n");
				fprintf(fp_output, "\tfldz\n");
				fprintf(fp_output, "\tfcompp\n");
			}
			else
			{
				fprintf(fp_output, "\n\t; popping from stack\n");
				width /= 2;
				while (width--)
					fprintf(fp_output, "\tpop ax\n");
			}
		}
	}


	fprintf(fp_output, "\n.exit:\n");
	fprintf(fp_output, "\tadd rsp, %d\n", localSize);
	fprintf(fp_output, "\tpop rbp\n");
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
		fprintf(fp, "\tfinit\n");
		handleFunction(functions[i++]);
		fprintf(fp_output, "\tret\n");
		func = func->sibling;
	}

	func = rt->children[1];
	TypeLog* mediator = trie_getRef(globalSymbolTable, func->token->lexeme)->entry.ptr;
	entry = mediator->structure;
	localSymbolTable = entry->symbolTable;

	fprintf(fp, "\nmain:\n");
	fprintf(fp, "\tfinit\n");
	handleFunction(functions[i++]);
	fprintf(fp, "\tmov rax, 0\n");
	fprintf(fp, "\tret\n");
	fprintf(fp, "\nsection .data\n");
	fprintf(fp, "\tint_out:  db  \"%chd\", 10, 0\n", '%');
	fprintf(fp, "\tint_in:  db  \"%chd\", 0\n", '%');
	fprintf(fp, "\treal_out:  db  \"%cf\", 10, 0\n", '%');
	fprintf(fp, "\treal_in:  db  \"%cf\", 0\n", '%');
	fprintf(fp, "\treal_val:  db  0,0,0,0\n");

	iterateTrie(globalSymbolTable, fillDataSegment);
}