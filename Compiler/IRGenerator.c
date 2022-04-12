#include  "IRGenerator.h"
#include "trie.h"
#include <string.h>
#include <stdio.h>
#include "logger.h"

char* IRCodeLoc = "assembly_";
char buff[50];

int globalOffset = 0;
int localOffset = 0;
int labelCount = 1;
Trie* localTable;

void getNextTemp(char* buff)
{
	static int index = 0;
	strcpy(buff, "##");
	sprintf(buff + 2, "%d", index++);
}

void genAssembly(ASTNode* stmt)
{
	if (stmt->sym_index == 90)
	{
		genAssembly(stmt->children[1]); // smt->children[1] => arithmeticExpression
	}
	else if (stmt->token->type == TK_GE)
	{

	}
	else if (stmt->token->type == TK_LE)
	{

	}
	//genAssembly(stmt->);
}

void generateFunctionCode(ASTNode* node)
{
	localOffset = 0;
	labelCount = 1;
	localTable = ((FuncEntry*)((TypeLog*)trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr))->symbolTable;

	//node->children[0] => <input_pars>
	ASTNode* inputs = node->children[0];
	while (inputs)
	{
		VariableEntry * entry = inputs->derived_type->structure;
		entry->offset = localOffset;
		entry->isGlobal = 0;
		localOffset += inputs->derived_type->width;
		inputs = inputs->sibling;
	}

	//node->children[1] => <output_par>
	ASTNode* outputs = node->children[1];
	while (outputs)
	{
		VariableEntry* entry = outputs->derived_type->structure;
		entry->offset = localOffset;
		entry->isGlobal = 0;
		localOffset += outputs->derived_type->width;
		outputs = outputs->sibling;
	}

	//node->children[2] => stmts
	//stmts->children[1] => declarations
	ASTNode* vars = node->children[2]->children[1];
	while (vars)
	{
		VariableEntry* entry = vars->derived_type->structure;
		entry->offset = node->isGlobal ? globalOffset : localOffset;
		entry->isGlobal = node->isGlobal;

		if (node->isGlobal)
			globalOffset += vars->derived_type->width;
		else
			localOffset += vars->derived_type->width;

		vars = vars->sibling;
	}

	//stmts->children[2] => OtherStmts
	strcpy(buff, IRCodeLoc);
	strcpy(buff + strlen(IRCodeLoc), node->token->lexeme);
	FILE* fp = fopen(buff, "w");
	genAssembly(node->children[2]->children[2], node->token->lexeme);
	fclose(fp);
}