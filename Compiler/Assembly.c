#include "Assembly.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

ASTNode* func;
FuncEntry* entry;
Trie* localSymbolTable;
FILE* fp_output;

char* prefix = "section .text\n\tdefault rel\n\textern printf\n\textern scanf\n\tglobal main\n\n";

// Get  info
char** splitVariable(char* name)
{
	int argc = 1;
	char** argv = NULL;
	for (char* c = name; *c; ++c)
		argc += (*c == '.');

	argv = calloc(argc + 1, sizeof(char*));
	argc = 0;
	argv[argc++] = name;

	for (char* c = name; *c; ++c)
	{
		if (*c != '.')
			continue;

		argv[argc++] = c + 1;
		*c = '\0';
	}

	return argv;
}

TypeLog* getVarType(char* name)
{
	char** argv = splitVariable(name);

	TypeLog* entry = trie_getRef(localSymbolTable, argv[0])->entry.ptr;
	VariableEntry* varEntry;

	if (entry != NULL)
		varEntry = entry->structure;
	else
	{
		entry = trie_getRef(globalSymbolTable, argv[0])->entry.ptr;
		varEntry = entry->structure;
	}
	
	TypeLog* derType = varEntry->type;

	for (char** ptr = argv + 1; *ptr; ++ptr)
	{
		int found = 0;
		DerivedEntry* derEntry = derType->structure;

		// here, we need to search *ptr in derEntry
		for (TypeInfoListNode* node = derEntry->list->head; node; node = node->next)
		{
			if (strcmp(node->name, *ptr))
				continue;

			found = 1;
			derType = node->type;
		}

		assert(found);
	}

	for (char** ptr = argv + 1; *ptr; ++ptr)
		*(*ptr - 1) = '.';

	free(argv);
	return derType;
}

// ASM Name
char* getBase(char* name)
{
	char* dot = NULL;
	char old = '\0';
	for (dot = name; old = *dot, (*dot != '\0' && *dot != '.'); dot++);
	*dot = '\0';

	TypeLog* entry = trie_getRef(localSymbolTable, name)->entry.ptr;
	char buff[50];

	if (entry != NULL)
		strcpy(buff, "rbp");
	else
		strcpy(buff, name);

	char* ret = calloc(strlen(buff) + 1, sizeof(char));
	strcpy(ret, buff);
	*dot = old;
	return ret;
}

int getOffset(char* name)
{
	char** argv = splitVariable(name);

	int baseOffset = 0;
	TypeLog* entry = trie_getRef(localSymbolTable, argv[0])->entry.ptr;
	VariableEntry* varEntry;

	if (entry != NULL)
	{
		varEntry = entry->structure;
		baseOffset -= varEntry->offset + varEntry->type->width;
	}
	else
	{
		entry = trie_getRef(globalSymbolTable, argv[0])->entry.ptr;
		varEntry = entry->structure;
		baseOffset = 0;
	}
	
	DerivedEntry* derEntry = varEntry->type->structure;

	for (char** ptr = argv + 1; *ptr; ++ptr)
	{
		int found = 0;
		int local_offset = 0;

		// here, we need to search *ptr in derEntry
		for (TypeInfoListNode* node = derEntry->list->head; node; node = node->next)
		{
			if (strcmp(node->name, *ptr))
			{
				local_offset += derEntry ? (derEntry->isUnion ? 0 : node->type->width) : node->type->width;
				continue;
			}

			found = 1;
			baseOffset += local_offset;
			derEntry = node->type->structure;
		}

		assert(found);
	}

	for (char** ptr = argv + 1; *ptr; ++ptr)
		*(*ptr - 1) = '.';

	free(argv);
	return baseOffset;
}

char* getReferenceName(char* name)
{
	char* base = getBase(name);
	int offset = getOffset(name);
	char* ret = calloc(50, sizeof(char));

	sprintf(ret, "[%s %c", base, offset > 0 ? '+' : '-');
	sprintf(ret + strlen(ret), " %d]", offset > 0 ? offset : -offset);
	free(base);
	return ret;
}

// Suffix
void fillDataSegment(char* key, TrieEntry* entry)
{
	TypeLog* typelog = entry->ptr;

	if (typelog->entryType != VARIABLE)
		return;
	
	VariableEntry* var = typelog->structure;
	fprintf(fp_output, "\t%s:\tdb\t%d\tdup(0)\n", var->name, var->type->width);
	return;
}

// read
void readVariableRecursive(char* prefix, TypeLog* log)
{
	if (log->entryType == INT)
	{
		char* loc = getReferenceName(prefix);
		fprintf(fp_output, "\tlea rsi, %s\n", loc);
		fprintf(fp_output, "\tmov rdi, int_in\n");
		fprintf(fp_output, "\txor rax, rax\n");
		fprintf(fp_output, "\tcall scanf\n");
		free(loc);

		return;
	}

	if (log->entryType == REAL)
	{
		char* loc = getReferenceName(prefix);
		fprintf(fp_output, "\tlea rsi, %s\n", loc);
		fprintf(fp_output, "\tmov rdi, real_in\n");
		fprintf(fp_output, "\txor rax, rax\n");
		fprintf(fp_output, "\tcall scanf\n");
		free(loc);

		return;
	}

	assert(log->entryType == DERIVED);

	DerivedEntry* d = log->structure;

	if (d->isUnion)
	{
		char* end = prefix + strlen(prefix);
		strcpy(end, d->list->head->name);
		readVariableRecursive(prefix, d->list->head->type);
		*end = '\0';
		
		return;
	}

	for (TypeInfoListNode* node = d->list->head; node; node = node->next)
	{
		char* end = prefix + strlen(prefix);
		strcpy(end, ".");
		strcpy(end + 1, node->name);
		readVariableRecursive(prefix, node->type);

		*end = '\0';
	}
}

void readVariable(char* name)
{
	fprintf(fp_output, "\n\t; Reading variable: %s\n", name);

	char* prefix = calloc(50, sizeof(char));
	strcpy(prefix, name);
	readVariableRecursive(prefix, getVarType(name));
	free(name);
}

// write
void writeVariableRecursive(char* prefix, TypeLog* log)
{
	if (log->entryType == INT)
	{
		char* loc = getReferenceName(prefix);
		fprintf(fp_output, "\tmov ax, word %s\n", loc);
		fprintf(fp_output, "\tmovsx rsi, ax\n");
		fprintf(fp_output, "\tmov rdi, int_out\n");
		fprintf(fp_output, "\txor rax, rax\n");
		fprintf(fp_output, "\tcall printf\n");
		free(loc);

		return;
	}

	if (log->entryType == REAL)
	{
		char* loc = getReferenceName(prefix);
		fprintf(fp_output, "\tcvtss2sd xmm0, %s\n", loc);
		fprintf(fp_output, "\tmov rdi, real_out\n");
		fprintf(fp_output, "\tmov rax, 1\n");
		fprintf(fp_output, "\tcall printf\n");
		free(loc);

		return;
	}

	assert(log->entryType == DERIVED);

	DerivedEntry* d = log->structure;

	if (d->isUnion)
	{
		char* end = prefix + strlen(prefix);
		strcpy(end, ".");
		strcpy(end + 1, d->list->head->name);
		writeVariableRecursive(prefix, d->list->head->type);
		*end = '\0';

		return;
	}

	for (TypeInfoListNode* node = d->list->head; node; node = node->next)
	{
		char* end = prefix + strlen(prefix);
		strcpy(end, ".");
		strcpy(end + 1, node->name);
		writeVariableRecursive(prefix, node->type);

		*end = '\0';
	}
}

void writeVariable(char* name)
{
	fprintf(fp_output, "\n\t; Writing variable: %s\n", name);

	char* prefix = calloc(50, sizeof(char));
	strcpy(prefix, name);
	writeVariableRecursive(prefix, getVarType(name));
	free(name);

	fprintf(fp_output, "\tmov rsi, 0\n");
	fprintf(fp_output, "\tmov rdi, newline\n");
	fprintf(fp_output, "\txor rax, rax\n");
	fprintf(fp_output, "\tcall printf\n");
}

// Arith

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

			if (getVarType(instr->src1.name)->entryType == REAL)
				fprintf(fp_output, "\tfstp dword %s\n", loc);
			else
			{
				char* lexeme = instr->src1.name;
				TypeLog* varType = getVarType(lexeme);

				fprintf(fp_output, "\tmov rdi, %s\n", getBase(lexeme));
				fprintf(fp_output, "\tadd rdi, %d\n", getOffset(lexeme));
				fprintf(fp_output, "\tmov rsi, rsp\n");
				fprintf(fp_output, "\tmov rcx, %d\n", varType->width);
				fprintf(fp_output, "\trepnz movsb\n\n");
				fprintf(fp_output, "\tadd rsp, %d\n", varType->width);
			}

			free(loc);
		}
		else if (instr->op == OP_CALL)
		{
			ASTNode* ast = instr->src1.astnode;

			fprintf(fp_output, "\n\t; Function call to %s\n", ast->token->lexeme);

			int offset = 0;

			for (
				ASTNode* out = ast->children[0], *in = ast->children[1];
				out || in;
				(out) ? (out = out->sibling) : (in = in->sibling))
			{
				char* lexeme = out ? out->token->lexeme : in->token->lexeme;
				TypeLog* varType = getVarType(lexeme);

				fprintf(fp_output, "\tmov rsi, %s\n", getBase(lexeme));
				fprintf(fp_output, "\tadd rsi, %d\n", getOffset(lexeme));
				fprintf(fp_output, "\tmov rdi, rsp\n");
				fprintf(fp_output, "\tsub rdi, %d\n", (offset += varType->width) + 16);
				fprintf(fp_output, "\tmov rcx, %d\n", varType->width);
				fprintf(fp_output, "\trepnz movsb\n\n");
			}

			fprintf(fp_output, "\n\tcall %s\n", ast->token->lexeme);

			// get the result back
			offset = 0;
			for (
				ASTNode* out = ast->children[0], *in = ast->children[1];
				out || in;
				(out) ? (out = out->sibling) : (in = in->sibling))
			{
				char* lexeme = out ? out->token->lexeme : in->token->lexeme;
				TypeLog* varType = getVarType(lexeme);

				fprintf(fp_output, "\tmov rdi, %s\n", getBase(lexeme));
				fprintf(fp_output, "\tadd rdi, %d\n", getOffset(lexeme));
				fprintf(fp_output, "\tmov rsi, rsp\n");
				fprintf(fp_output, "\tsub rsi, %d\n", (offset += varType->width) + 16);
				fprintf(fp_output, "\tmov rcx, %d\n", varType->width);
				fprintf(fp_output, "\trepnz movsb\n\n");
			}
		}
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
			if (getVarType(instr->src1.name)->entryType == REAL)
			{
				fprintf(fp_output, "\tfld dword %s\n", loc);
			}
			else 
			{
				char* lexeme = instr->src1.name;
				TypeLog* varType = getVarType(lexeme);
				fprintf(fp_output, "\tmov rsi, %s\n", getBase(lexeme));
				fprintf(fp_output, "\tadd rsi, %d\n", getOffset(lexeme));
				fprintf(fp_output, "\tsub rsp, %d\n", varType->width);
				fprintf(fp_output, "\tmov rdi, rsp\n");
				fprintf(fp_output, "\tmov rcx, %d\n", varType->width);
				fprintf(fp_output, "\trepnz movsb\n\n");
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
		entry = mediator->structure;
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
	fprintf(fp, "\tint_out:  db  \"%chd\", 09, 0\n", '%');
	fprintf(fp, "\tint_in:  db  \"%chd\", 0\n", '%');
	fprintf(fp, "\treal_out:  db  \"%cf\", 09, 0\n", '%');
	fprintf(fp, "\treal_in:  db  \"%cf\", 0\n", '%');
	fprintf(fp, "\tnewline:  db  10, 0\n");
	fprintf(fp, "\treal_val:  db  0,0,0,0\n");

	iterateTrie(globalSymbolTable, fillDataSegment);
}