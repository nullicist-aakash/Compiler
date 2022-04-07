#pragma once
#include "ast.h"

typedef enum
{
    INT,
    REAL,
    FUNCTION,
    DERIVED,
    BOOL,
    VOID
} TypeTag;

typedef struct TypeInfo
{
    int index;
    TypeTag entryType;
    int width; // Memory to allocate to variable of this type
    void* structure; // Pointer to type information
} TypeInfo;

// Acts as an intermediate, which helps for type definitions
typedef struct TypeLog
{
    int refCount;
    struct TypeInfo entry;
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

typedef struct
{
    struct TypeInfoList *argTypes;
    struct TypeInfoList *retTypes;
} FuncEntry;

typedef struct
{
    //TODO : Check if necessary before code generation
    int isUnion;
    char *name; // Name of record/union
    TypeInfoList *list;
} DerivedEntry;

void loadSymbolTable(ASTNode *);