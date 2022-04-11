#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "symbolTable.h"
#include "trie.h"
#include "toposort.h"
#include "logger.h"

typedef enum
{
    NAME_REDEFINED,
    NON_EXISTENT_TYPE,
    NAME_TYPE_MISMTACH,
    REDEFINED_ALIAS,
    UNDEFINED_TYPE,
    INVALID_TYPE,
    DUPLICATE_FIELD_NAME,
} ErrorType;

typedef struct ErrorListNode
{
    ErrorType etype;
    ASTNode *errnode;
    struct ErrorListNode *next;
} ErrorListNode;

typedef struct
{
    ErrorListNode *head;
    ErrorListNode *tail;
} ErrorList;

Trie *globalSymbolTable; // Stores information about records and unions
Trie *prefixTable;       // Stores the type of defined structure (record/union/typedef)

int dataTypeCount = 0;
int identifierCount = 0; // both function and variables
ErrorList *errList;
TypeLog **structList;

// First pass : Collect struct , typedef and pass through typedef list
// Second pass : Add function, Fill TypeInfo, fill variable infos

void initTables()
{
    globalSymbolTable = calloc(1, sizeof(Trie));

    TypeLog *intInfo = calloc(1, sizeof(TypeLog));
    intInfo->refCount = 1;
    intInfo->entryType = INT;
    intInfo->width = 8;
    intInfo->index = dataTypeCount++;

    trie_getRef(globalSymbolTable, "int")->entry.ptr = intInfo;

    TypeLog *realInfo = calloc(1, sizeof(TypeLog *));
    realInfo->refCount = 1;
    realInfo->entryType = REAL;
    realInfo->width = 8;
    realInfo->index = dataTypeCount++;

    trie_getRef(globalSymbolTable, "real")->entry.ptr = realInfo;

    TypeLog *boolInfo = calloc(1, sizeof(TypeLog *));
    boolInfo->refCount = 1;
    boolInfo->entryType = BOOL;
    boolInfo->width = 8;
    boolInfo->index = dataTypeCount++;

    trie_getRef(globalSymbolTable, "##bool")->entry.ptr = boolInfo;

    TypeLog *voidInfo = calloc(1, sizeof(TypeLog *));
    voidInfo->refCount = 1;
    voidInfo->entryType = VOID;
    voidInfo->width = 8;
    voidInfo->index = dataTypeCount++;

    trie_getRef(globalSymbolTable, "##void")->entry.ptr = voidInfo;
    prefixTable = calloc(1, sizeof(Trie));
}

TypeLog *getMediator(Trie *t, char *key)
{
    TrieNode *node = trie_getRef(t, key);

    if (node->entry.value == 0)
        node->entry.ptr = calloc(1, sizeof(TypeLog));

    return (TypeLog *)node->entry.ptr;
}

void addToErrorList(ASTNode *node, ErrorType type)
{
    if (!errList->head)
        errList->head = errList->tail = calloc(1, sizeof(ErrorListNode));
    else
    {
        errList->tail->next = calloc(1, sizeof(ErrorListNode));
        errList->tail = errList->tail->next;
    }

    errList->tail->etype = type;
    errList->tail->errnode = node;
}

int firstPassErrorCheck(ASTNode *node)
{
    if (node->token->type != TK_DEFINETYPE)
    {
        if (trie_exists(prefixTable, node->children[0]->token->lexeme))
        {
            addToErrorList(node, NAME_REDEFINED);
            return -1;
        }
    }

    // TODO : Errors
    // 1.1 Type Name Redefined
    // 1.2.1 Non Existent type for Alias
    // 1.2.2 Alias type mismatch
    // 1.2.3 Redefined alias name
    // 1.3.1 Func Name Redefined

    return 0;
}

int secondPassErrorCheck(ASTNode *node)
{
    // TODO : Errors
    /* For Functions
        1. Invalid argument type
        2. Repeated variable name
    */
    return 0;
}

void iterationFunction(TrieEntry *entry)
{
    TypeLog *typelog = entry->ptr;

    if (typelog->entryType == INT)
        logIt("%s\n", "int");

    else if (typelog->entryType == REAL)
        logIt("%s\n", "real");

    else if (typelog->entryType == BOOL)
        logIt("%s\n", "bool");

    else if (typelog->entryType == VOID)
        logIt("%s\n", "void");

    else if (typelog->entryType == FUNCTION)
    {
        FuncEntry *func = typelog->structure;
        logIt("Function Name: %s and is stored at address %p\n", func->name, func);
        logIt("\tinput: ");

        TypeInfoListNode *hd = func->argTypes->head;
        while (hd)
        {
            logIt(" { %s: %s } at %p", hd->name,
                  hd->type->entryType == INT ? "int" : hd->type->entryType == REAL ? "real"
                                                                                   : ((DerivedEntry *)hd->type->structure)->name,
                  hd->type->structure);

            hd = hd->next;
        }

        logIt("\n\toutput: ");

        hd = func->retTypes->head;
        while (hd)
        {
            logIt("{ %s: %s } at %p", hd->name,
                  hd->type->entryType == INT ? "int" : hd->type->entryType == REAL ? "real"
                                                                                   : ((DerivedEntry *)hd->type->structure)->name,
                  hd->type->structure);

            hd = hd->next;
        }
        logIt("\n");
        logIt("\trefCount = %d, index: %d, width: %d\n\n", typelog->refCount, typelog->index, typelog->width);
        iterateTrie(func->symbolTable, iterationFunction);

        logIt("Done with function\n");
    }
    else if (typelog->entryType == DERIVED)
    {
        DerivedEntry *entry = typelog->structure;

        logIt("%s %s and is stored at address %p\n", entry->isUnion ? "union" : "record", entry->name, entry);

        if (!entry->list)
            return;

        TypeInfoListNode *hd = entry->list->head;
        while (hd)
        {
            logIt(" { %s: %s } at %p ", hd->name,
                  hd->type->entryType == INT ? "int" : hd->type->entryType == REAL ? "real"
                                                                                   : ((DerivedEntry *)hd->type->structure)->name,
                  hd->type->structure);

            hd = hd->next;
        }

        logIt("\n");
        logIt("\trefCount = %d, index: %d, width: %d\n\n", typelog->refCount, typelog->index, typelog->width);
    }
    else if (typelog->entryType == VARIABLE)
    {
        VariableEntry *var = typelog->structure;
        logIt("variable %s is of type %s and is stored at address %p\n", var->name,
              var->type->entryType == INT ? "int" : var->type->entryType == REAL ? "real"
                                                                                 : ((DerivedEntry *)var->type->structure)->name,
              var);
        logIt("\trefCount = %d, index: %d, width: %d\n\n", typelog->refCount, typelog->index, ((VariableEntry *)typelog->structure)->type->width);
    }
}

void firstPass(ASTNode *node)
{
    if (!node)
        return;

    if (node->sym_index == 57)
    {
        // <program> -> <funcList> <mainFunction>
        firstPass(node->children[0]);
        firstPass(node->children[1]);
    }
    else if (node->sym_index == 60 || node->sym_index == 58) // Function names parsed
    {
        // <function> -> <inputList><outputList> <stmts>

        TypeLog *mediator = getMediator(globalSymbolTable, node->token->lexeme);
        mediator->refCount = 1;
        mediator->entryType = FUNCTION;
        mediator->width = -1;
        mediator->index = identifierCount++;

        FuncEntry *entry = calloc(1, sizeof(FuncEntry));
        mediator->structure = entry;
        entry->name = calloc(node->token->length + 1, sizeof(char));
        strcpy(entry->name, node->token->lexeme);
        entry->identifierCount = 0;
        entry->argTypes = calloc(1, sizeof(TypeInfoList));
        entry->retTypes = calloc(1, sizeof(TypeInfoList));
        entry->symbolTable = calloc(1, sizeof(Trie));
        firstPass(node->children[2]);
    }
    else if (node->sym_index == 68)
    {
        // <stmts> -> <definitions> <declarations> <funcBody> <return>
        firstPass(node->children[0]);
    }
    else if (node->sym_index == 71 && firstPassErrorCheck(node) != -1) // Type Definition Names Parsed
    {
        trie_getRef(prefixTable, node->children[0]->token->lexeme)->entry.value =
            node->token->type;

        TypeLog *mediator = getMediator(globalSymbolTable, node->children[0]->token->lexeme);
        mediator->structure = calloc(1, sizeof(DerivedEntry));
        mediator->refCount = 1;
        mediator->entryType = DERIVED;
        mediator->width = -1;
        mediator->index = dataTypeCount++;
    }
    else if (node->sym_index == 108 && firstPassErrorCheck(node) != -1) // Type Aliases Parsed
    {
        char *oldName = node->children[1]->token->lexeme;
        char *newName = node->children[2]->token->lexeme;

        TypeLog *mediator = getMediator(globalSymbolTable, oldName);
        mediator->refCount++;
        trie_getRef(globalSymbolTable, newName)->entry.ptr = mediator;

        trie_getRef(prefixTable, newName)->entry.value = node->token->type;
    }

    firstPass(node->sibling);
}

FuncEntry *local_func;
void secondPass(ASTNode *node, int **adj, Trie *symTable)
{
    if (!node)
        return;

    if (node->sym_index == 57)
    {
        // <program> -> <funcList> <mainFunction>
        secondPass(node->children[0], adj, symTable);
        secondPass(node->children[1], adj, symTable);
    }
    else if (node->sym_index == 60 || node->sym_index == 58) // Function Type Parsed
    {
        // <function> -> <inputList><outputList> <stmts>
        // Fill input argument

        TypeLog *mediator = getMediator(symTable, node->token->lexeme);
        FuncEntry *entry = mediator->structure;

        ASTNode *arg = node->children[0];
        while (arg)
        {
            if (!entry->argTypes->head)
                entry->argTypes->head = entry->argTypes->tail = calloc(1, sizeof(TypeInfoListNode));
            else
            {
                entry->argTypes->tail->next = calloc(1, sizeof(TypeInfoListNode));
                entry->argTypes->tail = entry->argTypes->tail->next;
            }

            // TODO: Check error
            entry->argTypes->tail->type = (arg->type->sibling ? getMediator(globalSymbolTable, arg->type->sibling->token->lexeme) : getMediator(globalSymbolTable, arg->type->token->lexeme));
            entry->argTypes->tail->type->refCount++;
            entry->argTypes->tail->name = calloc(arg->token->length + 1, sizeof(char));
            strcpy(entry->argTypes->tail->name, arg->token->lexeme);

            arg = arg->sibling;
        }

        ASTNode *ret = node->children[1];

        while (ret)
        {
            if (!entry->retTypes->head)
                entry->retTypes->head = entry->retTypes->tail = calloc(1, sizeof(TypeInfoListNode));
            else
            {
                entry->retTypes->tail->next = calloc(1, sizeof(TypeInfoListNode));
                entry->retTypes->tail = entry->retTypes->tail->next;
            }

            // TODO: Check error
            entry->retTypes->tail->type = (ret->type->sibling ? getMediator(globalSymbolTable, ret->type->sibling->token->lexeme) : getMediator(globalSymbolTable, ret->type->token->lexeme));
            entry->retTypes->tail->type->refCount++;
            entry->retTypes->tail->name = calloc(ret->token->length + 1, sizeof(char));
            strcpy(entry->retTypes->tail->name, ret->token->lexeme);

            ret = ret->sibling;
        }

        local_func = entry;
        secondPass(node->children[0], adj, local_func->symbolTable);
        secondPass(node->children[1], adj, local_func->symbolTable);
        secondPass(node->children[2], adj, local_func->symbolTable);
    }
    else if (node->sym_index == 68)
    {
        // <stmts> -> <definitions> <declarations> <funcBody> <return>

        secondPass(node->children[0], adj, symTable);

        ASTNode *declarationNode = node->children[1];
        secondPass(declarationNode, adj, symTable);
    }
    else if (node->sym_index == 71 && secondPassErrorCheck(node) != -1) // Record/Union full type information parsed
    {
        // <typeDefinition> -> TK_RUID <fieldDefinitions>
        ASTNode *field = node->children[1];
        TypeLog *mediator = getMediator(globalSymbolTable, node->children[0]->token->lexeme);

        DerivedEntry *entry = mediator->structure;
        entry->isUnion = node->token->type == TK_UNION;
        entry->name = calloc(node->children[0]->token->length + 1, sizeof(char));
        strcpy(entry->name, node->children[0]->token->lexeme);

        entry->list = calloc(1, sizeof(TypeInfoList));

        structList[mediator->index] = mediator;

        while (field)
        {
            if (!entry->list->head)
                entry->list->head = entry->list->tail = calloc(1, sizeof(TypeInfoListNode));
            else
            {
                entry->list->tail->next = calloc(1, sizeof(TypeInfoListNode));
                entry->list->tail = entry->list->tail->next;
            }

            TypeInfoListNode *infoNode = entry->list->tail;

            // TODO: Check error
            infoNode->type = getMediator(globalSymbolTable, field->type->token->lexeme);
            infoNode->type->refCount++;
            infoNode->name = calloc(field->token->length + 1, sizeof(char));
            strcpy(infoNode->name, field->token->lexeme);

            adj[infoNode->type->index][mediator->index]++;
            field = field->sibling;
        }
    }
    else if ((node->sym_index == 63 || node->sym_index == 77) && secondPassErrorCheck(node) != -1)
    {
        // <declaration> ===> { token: TK_ID, type: <dataType> }
        // <dataType> ==> { TK_INT, TK_REAL, { TK_RECORD/TK_UNION, TK_RUID } }

        Trie *table = node->isGlobal ? globalSymbolTable : symTable;
        TypeLog *mediator = getMediator(table, node->token->lexeme);
        mediator->index = node->isGlobal ? identifierCount++ : local_func->identifierCount++;
        mediator->refCount++;
        mediator->entryType = VARIABLE;
        mediator->width = -1;

        mediator->structure = calloc(1, sizeof(VariableEntry));
        VariableEntry *entry = mediator->structure;

        entry->name = calloc(node->token->length + 1, sizeof(char));
        strcpy(entry->name, node->token->lexeme);
        entry->type = getMediator(globalSymbolTable, node->type->sibling == NULL ? node->type->token->lexeme : node->type->sibling->token->lexeme);
    }

    secondPass(node->sibling, adj, symTable);
}

void calculateWidth(int *sortedList, int index, int **adj)
{
    int width = 0;
    int actualIndex = sortedList[index];
    if (structList[actualIndex]->entryType == DERIVED)
    {
        DerivedEntry *t = structList[actualIndex]->structure;

        for (int i = 0; i < dataTypeCount; i++)
            width += adj[i][actualIndex] * structList[i]->width;
        structList[actualIndex]->width = width;
    }
}

void populateWidth(int **adj, int size)
{
    int *sortedList = calloc(size, sizeof(int));
    int err = topologicalSort(adj, sortedList, size);

    if (err == -1)
    {
        // TODO : Throw error when cycle detected
    }

    for (int i = 0; i < size; i++)
        calculateWidth(sortedList, i, adj);
}

void loadSymbolTable(ASTNode *root)
{
    errList = calloc(1, sizeof(ErrorList));

    initTables();

    logIt("========== Performing first pass ==========\n");
    firstPass(root);
    logIt("========== First pass done ==========\n");
    logIt("========== Printing result of first pass ==========\n");
    iterateTrie(globalSymbolTable, iterationFunction);
    logIt("========== Printing result of first pass done ==========\n");

    structList = calloc(dataTypeCount, sizeof(TypeLog *));
    structList[0] = trie_getRef(globalSymbolTable, "int")->entry.ptr;
    structList[1] = trie_getRef(globalSymbolTable, "real")->entry.ptr;
    structList[2] = trie_getRef(globalSymbolTable, "##bool")->entry.ptr;
    structList[3] = trie_getRef(globalSymbolTable, "##void")->entry.ptr;
    int **adj = calloc(dataTypeCount, sizeof(int *));
    for (int i = 0; i < dataTypeCount; i++)
        adj[i] = calloc(dataTypeCount, sizeof(int));

    logIt("========== Performing second pass ==========\n");
    secondPass(root, adj, globalSymbolTable);
    logIt("========== Second pass done ==========\n");

    populateWidth(adj, dataTypeCount);

    logIt("========== Printing result of second pass ==========\n");
    iterateTrie(globalSymbolTable, iterationFunction);
    logIt("========== Printing result of second pass done ==========\n");
}

int printErrors()
{
    return errList->head == NULL ? -1 : 0;
}