#pragma once
#include "ast.h"

typedef enum
{
    INT,
    REAL,
    DERIVED,
    BOOL,
    VOID,
    FUNCTION,
    VARIABLE
} TypeTag;

// Acts as an intermediate, which helps for type definitions
typedef struct TypeLog
{
    int refCount;
    int index;
    TypeTag entryType;
    int width; // Memory to allocate to variable of this type
    void* structure; // Pointer to type information
} TypeLog;

// List of types in Rec or Union
typedef struct TypeInfoListNode
{
    char *name;
    struct TypeLog *type; // Points to information of current Type
    struct TypeInfoListNode *next;
} TypeInfoListNode;

typedef struct TypeInfoList
{
    struct TypeInfoListNode* head;
    struct TypeInfoListNode* tail;
}TypeInfoList;

typedef struct VariableEntry
{
    int offset;
    int isGlobal;
    char* name;
    struct TypeLog* type;
} VariableEntry;

typedef struct
{
    char* name;
    int identifierCount;
    struct TypeInfoList *argTypes;
    struct TypeInfoList *retTypes;
    Trie* symbolTable;
} FuncEntry;

typedef struct
{
    //TODO : Check if necessary before code generation
    int isUnion;
    char *name; // Name of record/union
    TypeInfoList *list;
} DerivedEntry;

extern Trie *globalSymbolTable;        // Stores information about records and unions
extern Trie* prefixTable;      // Stores the type of defined structure (record/union/typedef)

extern int dataTypeCount;
extern int identifierCount;        // both function and variables

void loadSymbolTable(ASTNode *);

int printErrors();