#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "symbolTable.h"
#include "trie.h"
#include "toposort.h"
#include "logger.h"

Trie *globalSymbolTable; // Stores information about records and unions
Trie *prefixTable;       // Stores the type of defined structure (record/union/typedef)

int dataTypeCount = 0;
int identifierCount = 0; // both function and variables
TypeLog **structList;

// First pass : Collect struct , typedef and pass through typedef list
// Second pass : Add function, Fill TypeInfo, fill variable infos


int localOffset;
int globalOffset = 0;
Trie* localSymbolTable;

void fillOffsets(ASTNode* vars, Trie* table)
{
    while (vars)
    {
        TypeLog* mediator = trie_getRef(localSymbolTable, vars->token->lexeme)->entry.ptr;

        if (mediator == NULL)
            mediator = trie_getRef(globalSymbolTable, vars->token->lexeme)->entry.ptr;

        VariableEntry* varEntry = mediator->structure;

        varEntry->isGlobal = vars->isGlobal;
        varEntry->offset = varEntry->isGlobal ? globalOffset : localOffset;
        localOffset += varEntry->isGlobal ? 0 : varEntry->type->width;
        globalOffset += varEntry->isGlobal ? varEntry->type->width : 0;
        
        vars = vars->sibling;
    }
}

void generateFuncOffsets(ASTNode* funcNode)
{
    TypeLog* mediator = trie_getRef(globalSymbolTable, funcNode->token->lexeme)->entry.ptr;
    FuncEntry* funcEntry = mediator->structure;
    localSymbolTable = funcEntry->symbolTable;
    localOffset = 0;
    // Iterate over statements
    fillOffsets(funcNode->children[0], localSymbolTable);
    fillOffsets(funcNode->children[1], localSymbolTable);
    fillOffsets(funcNode->children[2]->children[1], localSymbolTable);

    funcEntry->activationRecordSize = localOffset;
}

void calculateOffsets(ASTNode* ast)
{
    // TODO: Encorporate this in typechecking (3rd pass)
    globalOffset = 0;
    ASTNode* func = ast->children[0] == NULL ? ast->children[1] : ast->children[0];
    while (func)
    {
        generateFuncOffsets(func);
        if (func->sibling == NULL && func != ast->children[1])
            func = ast->children[1];
        else
            func = func->sibling;
    }
}


void initTables()
{
    globalSymbolTable = calloc(1, sizeof(Trie));

    TypeLog *intInfo = calloc(1, sizeof(TypeLog));
    intInfo->refCount = 1;
    intInfo->entryType = INT;
    intInfo->width = 2;
    intInfo->index = dataTypeCount++;

    trie_getRef(globalSymbolTable, "int")->entry.ptr = intInfo;

    TypeLog *realInfo = calloc(1, sizeof(TypeLog *));
    realInfo->refCount = 1;
    realInfo->entryType = REAL;
    realInfo->width = 4;
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

int firstPassErrorCheck(ASTNode *node)
{
    if (node->token->type != TK_DEFINETYPE)
    {
        if (trie_exists(prefixTable, node->children[0]->token->lexeme))
        {
            // TODO: Name redefined error
            return -1;
        }
    }

    // TODO: : Errors
    // 1.1 Type Name Redefined
    // 1.2.1 Non Existent type for Alias
    // 1.2.2 Alias type mismatch
    // 1.2.3 Redefined alias name
    // 1.3.1 Func Name Redefined

    return 0;
}

int secondPassErrorCheck(ASTNode *node)
{
    // TODO: : Errors
    /* For Functions
        1. Invalid argument type
        2. Repeated variable name
    */
    return 0;
}

FuncEntry *local_func;  // TODO: Udao isko bc, use key

// void iterationFunction(char* key, TrieEntry *entry)
// {
//     TypeLog *typelog = entry->ptr;

//     if (typelog->entryType == INT)
//         logIt("%s\n", "int");

//     else if (typelog->entryType == REAL)
//         logIt("%s\n", "real");

//     else if (typelog->entryType == BOOL)
//         logIt("%s\n", "bool");

//     else if (typelog->entryType == VOID)
//         logIt("%s\n", "void");

//     else if (typelog->entryType == FUNCTION)
//     {
//         FuncEntry *func = typelog->structure;
//         logIt("Function Name: %s and is stored at address %p\n", func->name, func);
//         logIt("\tinput: ");

//         TypeInfoListNode *hd = func->argTypes->head;
//         while (hd)
//         {
//             logIt(" { %s: %s } at %p", hd->name,
//                 hd->type->entryType == INT ? "int" : 
//                 hd->type->entryType == REAL ? "real":
//                 ((DerivedEntry *)hd->type->structure)->name, hd->type->structure);

//             hd = hd->next;
//         }

//         logIt("\n\toutput: ");

//         hd = func->retTypes->head;
//         while (hd)
//         {
//             logIt("{ %s: %s } at %p", hd->name,
//                 hd->type->entryType == INT ? "int" : 
//                 hd->type->entryType == REAL ? "real":
//                 ((DerivedEntry *)hd->type->structure)->name, hd->type->structure);

//             hd = hd->next;
//         }
//         logIt("\n");
//         logIt("\trefCount = %d, index: %d, width: %d\n\n", typelog->refCount, typelog->index, typelog->width);
//         iterateTrie(func->symbolTable, iterationFunction);

//         logIt("Done with function\n");
//     }
//     else if (typelog->entryType == DERIVED)
//     {
//         DerivedEntry *entry = typelog->structure;

//         logIt("%s %s and is stored at address %p\n", entry->isUnion ? "union" : "record", entry->name, entry);

//         if (!entry->list)
//             return;

//         TypeInfoListNode *hd = entry->list->head;
//         while (hd)
//         {
//             logIt(" { %s: %s } at %p ", hd->name,
//                   hd->type->entryType == INT ? "int" : hd->type->entryType == REAL ? "real"
//                                                                                    : ((DerivedEntry *)hd->type->structure)->name,
//                   hd->type->structure);

//             hd = hd->next;
//         }

//         logIt("\n");
//         logIt("\trefCount = %d, index: %d, width: %d\n\n", typelog->refCount, typelog->index, typelog->width);
//     }
//     else if (typelog->entryType == VARIABLE)
//     {
//         VariableEntry *var = typelog->structure;
//         logIt("variable %s is of type %s and is stored at address %p\n", var->name,
//               var->type->entryType == INT ? "int" : var->type->entryType == REAL ? "real"
//                                                                                  : ((DerivedEntry *)var->type->structure)->name,
//               var);
//         logIt("\trefCount = %d, index: %d, width: %d\n\n", typelog->refCount, typelog->index, ((VariableEntry *)typelog->structure)->type->width);
//     }
// }

int firstPass(ASTNode *node)
{
    if (!node)
        return 0;
    if (node->sym_index == -1)
        firstPass(node->sibling);
    else if (node->sym_index == 57)
    {
        // <program> -> <funcList> <mainFunction>
        firstPass(node->children[0]);
        firstPass(node->children[1]);
    }
    else if (node->sym_index == 60 || node->sym_index == 58) // Function names parsed
    {
        // <function> -> <inputList><outputList> <stmts>
        if (trie_exists(globalSymbolTable, node->token->lexeme)) // ERROR Function name repeated
        {
            printf("Redeclaration of function\n");
            node->sym_index = -1;
            firstPass(node->sibling);
            return -1;
        }

        TypeLog *mediator = getMediator(globalSymbolTable, node->token->lexeme);
        mediator->refCount = 1;
        mediator->entryType = FUNCTION;
        mediator->width = -1;
        mediator->index = identifierCount++;

        FuncEntry *entry = calloc(1, sizeof(FuncEntry));
        mediator->structure = entry;
        entry->name = node->token->lexeme;
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
        if (trie_exists(prefixTable, node->children[0]->token->lexeme))
        {
            printf("Redeclaration of defined type \n"); // ERROR Definted Type name repeated
            node->sym_index = -1;
            firstPass(node->sibling);
            return -1;
        }

        trie_getRef(prefixTable, node->children[0]->token->lexeme)->entry.value =
            node->token->type;

        TypeLog *mediator = getMediator(globalSymbolTable, node->children[0]->token->lexeme);
        mediator->structure = calloc(1, sizeof(DerivedEntry));
        mediator->refCount = 1;
        mediator->entryType = DERIVED;
        mediator->width = -1;
        mediator->structure = calloc(1, sizeof(DerivedEntry));
        DerivedEntry *entry = mediator->structure;
        entry->isUnion = node->token->type == TK_UNION;
        entry->name = node->children[0]->token->lexeme;
        entry->list = calloc(1, sizeof(TypeInfoList));
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
        // Fill input argument

        TypeLog *mediator = getMediator(globalSymbolTable, node->token->lexeme);
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
            // TODO:: Check error
            // Adding to function parameters
            entry->argTypes->tail->type = (arg->type->sibling ? getMediator(globalSymbolTable, arg->type->sibling->token->lexeme) : getMediator(globalSymbolTable, arg->type->token->lexeme));
            entry->argTypes->tail->type->refCount++;
            entry->argTypes->tail->name = arg->token->lexeme;

            // Adding to function symbol table
            // TODO: make this a function
            TypeLog *argMediator = getMediator(entry->symbolTable, arg->token->lexeme);
            argMediator->index = entry->identifierCount++;
            argMediator->refCount = 1;
            argMediator->entryType = VARIABLE;
            argMediator->width = -1;

            argMediator->structure = calloc(1, sizeof(VariableEntry));
            VariableEntry *entry = argMediator->structure;

            entry->name = arg->token->lexeme;
            entry->isGlobal = 0;
            entry->usage = INPUT_PAR;
            entry->type = (arg->type->sibling ? getMediator(globalSymbolTable, arg->type->sibling->token->lexeme) : getMediator(globalSymbolTable, arg->type->token->lexeme));

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

            // TODO:: Check error
            entry->retTypes->tail->type = (ret->type->sibling ? getMediator(globalSymbolTable, ret->type->sibling->token->lexeme) : getMediator(globalSymbolTable, ret->type->token->lexeme));
            entry->retTypes->tail->name = ret->token->lexeme;

            TypeLog *retMediator = getMediator(entry->symbolTable, ret->token->lexeme);
            retMediator->index = entry->identifierCount++;
            retMediator->refCount = 1;
            retMediator->entryType = VARIABLE;
            retMediator->width = -1;

            retMediator->structure = calloc(1, sizeof(VariableEntry));
            VariableEntry *entry = retMediator->structure;

            entry->name = ret->token->lexeme;
            entry->isGlobal = 0;
            entry->usage = OUTPUT_PAR;
            entry->type = (ret->type->sibling ? getMediator(globalSymbolTable, ret->type->sibling->token->lexeme) : getMediator(globalSymbolTable, ret->type->token->lexeme));

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
        secondPass(node->children[1], adj, symTable);
    }
    else if (node->sym_index == 71 && secondPassErrorCheck(node) != -1) // Record/Union full type information parsed
    {
        ASTNode *field = node->children[1];
        TypeLog *mediator = getMediator(globalSymbolTable, node->children[0]->token->lexeme);

        DerivedEntry *entry = mediator->structure;

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

            // TODO:: Check error
            infoNode->type = getMediator(globalSymbolTable, field->type->token->lexeme);
            infoNode->type->refCount++;
            infoNode->name = field->token->lexeme;

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
        mediator->refCount = 1;
        mediator->entryType = VARIABLE;
        mediator->width = -1;

        mediator->structure = calloc(1, sizeof(VariableEntry));
        VariableEntry *entry = mediator->structure;

        entry->name = node->token->lexeme;
        entry->isGlobal = node->isGlobal;
        entry->usage = LOCAL;
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
        int isUnion = ((DerivedEntry *)(structList[actualIndex]->structure))->isUnion;
        DerivedEntry *t = structList[actualIndex]->structure;
        for (int i = 0; i < dataTypeCount; i++)
            width = isUnion ? (width > adj[i][actualIndex] * structList[i]->width ? width : adj[i][actualIndex] * structList[i]->width) : (width + adj[i][actualIndex] * structList[i]->width);
        structList[actualIndex]->width = width;
    }
}
void populateWidth(int **adj, int size)
{
    int *sortedList = calloc(size, sizeof(int));
    int err = topologicalSort(adj, sortedList, size);

    assert(err != -1);

    for (int i = 0; i < size; i++)
        calculateWidth(sortedList, i, adj);

    free(sortedList);

    for (int i = 0; i < dataTypeCount; i++)
        free(adj[i]);

    for (int i = 0; i < dataTypeCount; i++)
        free(structList[i]);
    free(structList);
    free(adj);
}

void printTypeName(VariableEntry *entry)
{
    TypeTag type = entry->type->entryType;
    if (type == INT || type == REAL)
        printf("---");
    else if (type == DERIVED)
    {
        DerivedEntry *de = entry->type->structure;
        AliasListNode *cur = de->aliases;
        printf("%s", de->name);
        while (cur)
        {
            printf(", %s", cur->RUName);
            cur = cur->next;
        }
    }
    printf("\n");
}

void printTypeExpression(TypeLog* entrytype)
{
    TypeTag type = entrytype->entryType;

    if (type == INT)
        printf("int");
    else if (type == REAL)
        printf("real");
    else if (type == DERIVED)
    {
        printf("<");
        DerivedEntry *de = entrytype->structure;
        TypeInfoListNode *cur = de->list->head;

        printTypeExpression(cur->type);
        cur = cur->next;
        while (cur)
        {
            printf(", ");
            printTypeExpression(cur->type);

            cur = cur->next;
        }
        printf(">");
    }
}

void printVariableUsage(VariableEntry *entry)
{
    if (entry->usage == LOCAL)
        printf("local");
    else if (entry->usage == INPUT_PAR)
        printf("input parameter");
    else if (entry->usage == OUTPUT_PAR)
        printf("output parameter");
    printf("\n");
}

void printGlobalSymbolTable(char* key, TrieEntry *entry)
{
    TypeLog *typelog = entry->ptr;
    if (typelog->entryType == VARIABLE)
    {
        VariableEntry *entry = typelog->structure;
        printf("Name - %s\n", entry->name);
        printf("Scope - Global\n");

        printf("Type Name - ");
        printTypeName(entry);

        printf("Type Expression - ");
        printTypeExpression(entry->type);
        printf("\n");

        printf("Width - %d\n", entry->type->width);
        printf("is Global - %s", entry->isGlobal ? "global\n" : "---\n");

        printf("Offset - %d\n", entry->offset);

        printf("Variable Usage - ");
        printVariableUsage(entry);
        printf("\n");
    }
    else
        return;
}

void printLocalTable(char* key, TrieEntry *entry)
{
    TypeLog *typelog = entry->ptr;
    if (typelog->entryType == VARIABLE)
    {
        VariableEntry *varentry = typelog->structure;
        printf("Name - %s\n", varentry->name);
        printf("Scope - %s\n", local_func->name);

        printf("Type Name - ");
        printTypeName(varentry);

        printf("Type Expression - ");
        printTypeExpression(varentry->type);
        printf("\n");

        printf("Width - %d\n", varentry->type->width);
        printf("isGlobal - %s", varentry->isGlobal ? "Global\n" : "---\n");

        printf("Offset - %d\n", varentry->offset);

        printf("Variable Usage - ");
        printVariableUsage(varentry);
        printf("\n");
    }
    else
        return;
}

void printFunctionSymbolTables(char* key, TrieEntry *entry)
{
    TypeLog *typelog = entry->ptr;
    if (typelog->entryType == FUNCTION)
    {
        local_func = (FuncEntry *)typelog->structure;
        iterateTrie(((FuncEntry *)typelog->structure)->symbolTable, printLocalTable);
    }
    else
        return;
}

void printFunctionActivationRecordSize(char* key, TrieEntry* entry)
{
    TypeLog* typelog = entry->ptr;
    if (typelog->entryType == FUNCTION)
    {
        local_func = (FuncEntry*)typelog->structure;
        printf("%s - %d\n", local_func->name, local_func->activationRecordSize);
    }
    else
        return;
}

void printRecordDetails(char* key, TrieEntry* entry)
{
    TypeLog* typelog = entry->ptr;
    if (typelog->entryType == DERIVED)
    {
        DerivedEntry* de = typelog->structure;
        printf("%s - ", key);
        printf("<");
        TypeInfoListNode* cur = de->list->head;

        printTypeExpression(cur->type);
        cur = cur->next;
        while (cur)
        {
            printf(", ");
            printTypeExpression(cur->type);

            cur = cur->next;
        }
        printf("> %d\n", typelog->width);
    }
    else
        return;
}

void loadSymbolTable(ASTNode *root)
{
    initTables();

    firstPass(root);
    iterateTrie(globalSymbolTable, iterationFunction);
    structList = calloc(dataTypeCount, sizeof(TypeLog *));
    structList[0] = trie_getRef(globalSymbolTable, "int")->entry.ptr;
    structList[1] = trie_getRef(globalSymbolTable, "real")->entry.ptr;
    structList[2] = trie_getRef(globalSymbolTable, "##bool")->entry.ptr;
    structList[3] = trie_getRef(globalSymbolTable, "##void")->entry.ptr;
    int **adj = calloc(dataTypeCount, sizeof(int *));
    for (int i = 0; i < dataTypeCount; i++)
        adj[i] = calloc(dataTypeCount, sizeof(int));

    secondPass(root, adj, globalSymbolTable);
    populateWidth(adj, dataTypeCount);

    calculateOffsets(root);
    // iterateTrie(globalSymbolTable, iterationFunction);
    dataTypeCount = 0;
}