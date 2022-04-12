#include "IRGenerator.h"
#include "logger.h"
#include <assert.h>

Trie* localSymbolTable;

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

IRInsList* generateFuncCode(ASTNode* funcNode)
{
	// get symbol table
	TypeLog* mediator = trie_getRef(globalSymbolTable, funcNode->token->lexeme)->entry.ptr;
	FuncEntry* funcEntry = mediator->structure;
	localSymbolTable = funcEntry->symbolTable;

	logIt("Generating code for function: %s and its symbol symbol is found at adress: %p\n", funcEntry->name, localSymbolTable);

	// Iterate over statements
	// TODO: Check if offsets are calculated
	return recurseiveGenFuncCode(funcNode->children[2]->children[2]);
}