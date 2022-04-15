#include "Assembly.h"
#include <assert.h>

ASTNode* func;
FuncEntry* entry;
Trie* localSymbolTable;
FILE* fp_output;

// Write globals
void writeVariables(TrieEntry* entry)
{
	TypeLog* typelog = entry->ptr;

	if (typelog->entryType != VARIABLE)
		return;

	VariableEntry* var = typelog->structure;

	fprintf(fp_output, "%s:\tdb\t%d\tdup(0)\n", var->name, var->type->width);
}

void handleFunction(IRInsNode* funcCode)
{
	// assuming this is main for now
	fprintf(fp_output, "\tpush ebp\n");
	fprintf(fp_output, "\tmov ebp, esp\n");
	fprintf(fp_output, "\tsub esp, %d\n", )
}

void generateAssembly(FILE* fp, ASTNode* rt, IRInsNode** functions)
{
	ASTNode* root = rt;
	fp_output = fp;

	fprintf(fp, "\tglobal main\n");
	fprintf(fp, "\textern printf\n");
	fprintf(fp, "\textern scanf\n");
	fprintf(fp, "\textern atoi\n");
	fprintf(fp, "\textern atof\n\n");
	fprintf(fp, "section .text\n");

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

	fprintf(fp, "main:\n");
	handleFunction(functions[i++]);

	fprintf(fp, "\tmov rax, 60                 ; system call for exit\n");
	fprintf(fp, "\txor rdi, rdi                ; exit code 0\n");
	fprintf(fp, "\tsyscall                     ; invoke operating system to exit\n\n");

	fprintf(fp, "section .data\n");

	iterateTrie(globalSymbolTable, writeVariables);
}