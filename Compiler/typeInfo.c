#include "typeInfo.h"
#include "string.h"

Trie *typeTable;

void init_typeTable()
{
	typeTable = calloc(1, sizeof(Trie));

	TypeInfo **intInfo = calloc(1, sizeof(TypeInfo *));
	*intInfo = calloc(1, sizeof(TypeInfo));
	(*intInfo)->entryType = INT;
	(*intInfo)->width = 4;

	trie_getRef(typeTable, "int")->entry.ptr = intInfo;

	TypeInfo **realInfo = calloc(1, sizeof(TypeInfo *));
	*realInfo = calloc(1, sizeof(TypeInfo));
	(*realInfo)->entryType = REAL;
	(*realInfo)->width = 4;

	trie_getRef(typeTable, "real")->entry.ptr = realInfo;
}

int addStructInfo(ASTNode *node)
{
	Token *name = node->children[0]->token;

	if (trie_exists(typeTable, name->lexeme) == 0)
		return -1;

	DerivedEntry *entry = calloc(1, sizeof(DerivedEntry));
	TypeInfo *newStruct = calloc(1, sizeof(TypeInfo));
	newStruct->entryType = DERIVED;
	newStruct->val = entry;

	if (strcmp("union", node->token->lexeme) == 0)
		entry->isUnion = 1;
	else
		entry->isUnion = 0;

	entry->name = calloc(name->length + 1, sizeof(char));
	strcpy(entry->name, name->lexeme);

	TypeInfoList *typeNode;
	ASTNode *fieldNode = node->children[1];

	while (fieldNode)
	{
		if (entry->list == NULL)
		{
			entry->list = calloc(1, sizeof(TypeInfoList));
			typeNode = entry->list;
		}
		else
		{
			typeNode->next = calloc(1, sizeof(TypeInfoList));
			typeNode = typeNode->next;
		}

		char *type = fieldNode->type->token->lexeme;

		typeNode->name = calloc(fieldNode->token->length + 1, sizeof(char));
		strcpy(typeNode->name, fieldNode->type->token->lexeme);

		typeNode->val = (TypeInfo **)trie_getRef(typeTable, type)->entry.ptr;

		fieldNode = fieldNode->sibling;
	}

	// TODO: I have feeling that this code will give some error but idk what it is before running it
	TypeInfo **entryToAdd = calloc(1, sizeof(TypeInfo *));
	*entryToAdd = newStruct;
	trie_getRef(typeTable, name->lexeme)->entry.ptr = entryToAdd;
	return 0;
}

int addTypedefInfo(ASTNode *node)
{
	Token *from = node->children[1]->token;
	Token *to = node->children[2]->token;

	if (trie_exists(typeTable, to->lexeme) == 0)
		return -1;
}