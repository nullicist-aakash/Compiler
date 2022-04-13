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

typedef enum
{
    LOCAL,
    INPUT_PAR,
    OUTPUT_PAR
    //GLOBAL
}VariableUsage;

// Acts as an intermediate, which helps for type definitions
typedef struct TypeLog
{
    int refCount;
    int index;
    TypeTag entryType;
    int width;       // Memory to allocate to variable of this type
    void *structure; // Pointer to type information
} TypeLog;

typedef struct VariableEntry
{
    int offset;
    int isGlobal;
    VariableUsage usage;
    char *name;
    struct TypeLog *type;
} VariableEntry;
// List of types in Rec or Union
typedef struct TypeInfoListNode
{
    char *name;
    struct TypeLog *type; // Points to information of current Type
    struct TypeInfoListNode *next;
} TypeInfoListNode;

typedef struct TypeInfoList
{
    struct TypeInfoListNode *head;
    struct TypeInfoListNode *tail;
} TypeInfoList;

typedef struct AliasListNode
{
    char *RUName;
    struct AliasListNode *next;
} AliasListNode;

typedef struct
{
    // TODO : Check if necessary before code generation
    int isUnion;
    char *name; // Name of record/union
    TypeInfoList *list;
    AliasListNode *aliases; // TODO : Remove the comment , but Aakash chutiya
} DerivedEntry;

typedef struct
{
    char *name;
    int identifierCount;
    struct TypeInfoList *argTypes;
    struct TypeInfoList *retTypes;
    Trie *symbolTable;
} FuncEntry;

extern Trie *globalSymbolTable; // Stores information about records and unions
extern Trie *prefixTable;       // Stores the type of defined structure (record/union/typedef)

extern int dataTypeCount;
extern int identifierCount; // both function and variables