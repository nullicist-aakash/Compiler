#pragma once
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
    struct TypeLog *type; // Points to information of current Type
    struct TypeInfoList *next;
} TypeInfoList;

typedef struct
{
    struct TypeInfoList *argTypes;
    struct TypeInfoList *retType;
    int index;
} FuncEntry;

typedef struct
{
    //TODO : Check if necessary before code generation
    int isUnion;
    char *name; // Name of record/union
    TypeInfoList *list;
} DerivedEntry;

typedef struct TypeInfo
{
    TypeTag entryType;
    int width; // Memory to allocate to variable of this type
    void *structure; // Pointer to type information
} TypeInfo;

void loadSymbolTable(ASTNode *);