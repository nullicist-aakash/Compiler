#pragma once
#include "Trie.h"
#include "AST.h"

typedef enum
{
	INT,
	REAL,
	FUNCTION,
	DERIVED,
	BOOL,
	VOID
} TypeTag;

struct TypeInfo;

typedef struct TypeInfoList
{
	char* name;
	struct TypeInfo** val;
	struct TypeInfoList* next;
} TypeInfoList;

typedef struct
{
	int argCount;
	struct TypeInfo*** argTypes;

	struct TypeInfo*** retType;
} FuncEntry;

typedef struct
{
	int isUnion;
	int isPrefixReq;
	char* name;
	TypeInfoList* list;
} DerivedEntry;

typedef struct TypeInfo
{
	TypeTag entryType;
	int width;
	void* val;
} TypeInfo;

extern Trie* typeTable;

void init_typeTable();
int addStructInfo(ASTNode*);
int addFunctionInfo(ASTNode*);
int addTypedefInfo(ASTNode*);