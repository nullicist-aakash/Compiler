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

// Acts as an intermediate, which helps for type definitions
typedef struct
{
	int refCount;
	TypeInfo entry;
} TypeLog;

// List of types in Rec or Union
typedef struct TypeInfoList
{
	char *name;
	struct TypeLog *val; // Points to information of current Type
	struct TypeInfoList *next;
} TypeInfoList;

typedef struct
{
	struct TypeInfoList *argTypes;
	struct TypeInfoList *retType;
} FuncEntry;

typedef struct
{
	int isUnion;
	char *name; // Name of record/union
	TypeInfoList *list;
} DerivedEntry;

typedef struct TypeInfo
{
	TypeTag entryType;
	int width; // Memory to allocate to variable of this type
	void *val; // Pointer to type information
} TypeInfo;

extern Trie *typeTable;

void init_typeTable();
int addStructInfo(ASTNode *);
int addFunctionInfo(ASTNode *);
int addTypedefInfo(ASTNode *);