#include "IRGenerator.h"
#include "logger.h"
#include <assert.h>

Trie* localSymbolTable;
int localOffset;
int globalOffset = 0;

IRInsList* mergeLists(IRInsList* left, IRInsList* right)
{
	if (!left->head)
		return right;

	if (!right->head)
		return left;

	left->tail->next = right->head;
	left->tail = right->tail;
	return left;
}

IRInsList* insert(IRInsList* list, Payload* ins)
{
	IRInsNode* temp = calloc(1, sizeof(IRInsNode));
	temp->ins = ins;

	if (!list->head)
		list->head = list->tail = temp;
	else
		list->tail->next = temp, list->tail = temp;

	return list;
}

int getNextLabel()
{
	static int label = 0;
	return ++label;
}

IRInsList* recurseiveGenFuncCode(ASTNode* stmt)
{
	assert(stmt != NULL);

	if (stmt->sym_index == 81)	// assignment
	{

	}
	else if (stmt->sym_index == 86) // funcCall
	{

	}
	else if (stmt->sym_index == 89) // iterative - while
	{

	}
	else if (stmt->sym_index == 90) // if-else
	{

	}
	else if (stmt->sym_index == 92)	// io
	{

	}
}

void fillOffsets(ASTNode* vars, FuncEntry* entry)
{
	while (vars)
	{
		VariableEntry* varEntry = vars->derived_type->structure;
		varEntry->offset = localOffset;
		varEntry->isGlobal = vars->isGlobal;
		localOffset += varEntry->isGlobal ? 0 : vars->derived_type->width;
		globalOffset += varEntry->isGlobal ? vars->derived_type->width : 0;

		vars = vars->sibling;
	}
}

IRInsList* generateFuncCode(ASTNode* funcNode)
{
	// get symbol table
	TypeLog* mediator = trie_getRef(globalSymbolTable, funcNode->token->lexeme)->entry.ptr;
	FuncEntry* funcEntry = mediator->structure;
	localSymbolTable = funcEntry->symbolTable;
	localOffset = 0;

	logIt("Generating code for function: %s and its symbol symbol is found at adress: %p\n", funcEntry->name, localSymbolTable);

	// Iterate over statements
	fillOffsets(funcNode->children[0], funcEntry);
	fillOffsets(funcNode->children[1], funcEntry);
	fillOffsets(funcNode->children[2]->children[1], funcEntry);
	return recurseiveGenFuncCode(funcNode->children[2]->children[2]);
}