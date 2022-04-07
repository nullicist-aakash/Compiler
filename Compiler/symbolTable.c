#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "symbolTable.h"
#include "trie.h"

typedef enum {
    NAME_REDEFINED,
    NON_EXISTENT_TYPE,
    NAME_TYPE_MISMTACH,
    REDEFINED_ALIAS,
    UNDEFINED_TYPE,
    INVALID_TYPE,
    DUPLICATE_FIELD_NAME,
} ErrorType;

typedef struct ErrorListNode {
    ErrorType etype;
    ASTNode* errnode;
    struct ErrorListNode* next;
} ErrorListNode;

typedef struct {
    ErrorListNode* head;
    ErrorListNode* tail;
} ErrorList;

Trie *typeTable;        // Stores information about records and unions
Trie* prefixTable;      // Stores the type of defined structure (record/union/typedef)
Trie* globalSymbolTable;// Stores everything global

int dataTypeCount = 0;
int funcCount = 0;
ErrorList* errList;

// First pass : Collect struct , typedef and pass through typedef list
// Second pass : Add function, Fill TypeInfo


void initTypeTable()
{
    typeTable = calloc(1, sizeof(Trie));

    TypeLog *intInfo = calloc(1, sizeof(TypeLog));
    intInfo->refCount = 1;
    intInfo->entry.entryType = INT;
    intInfo->entry.width = 4;
    intInfo->entry.index = dataTypeCount++;

    trie_getRef(typeTable, "int")->entry.ptr = intInfo;

    TypeLog *realInfo = calloc(1, sizeof(TypeLog *));
    realInfo->refCount = 1;
    realInfo->entry.entryType = REAL;
    realInfo->entry.width = 4;
    realInfo->entry.index = dataTypeCount++;

    trie_getRef(typeTable, "real")->entry.ptr = realInfo;
}

void initTables()
{
    initTypeTable();
    prefixTable = calloc(1, sizeof(Trie));
    globalSymbolTable = calloc(1, sizeof(Trie));

}

TypeLog* getMediator(char* key) 
{
    TrieNode* node = trie_getRef(typeTable, key);

    if (node->entry.value == 0)
        node->entry.ptr = calloc(1, sizeof(TypeLog));

    return (TypeLog*)node->entry.ptr;
}

void addToErrorList(ASTNode* node, ErrorType type) {
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

int firstPassErrorCheck(ASTNode* node) 
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

int secondPassErrorCheck(ASTNode* node) 
{


    
    // TODO : Errors 
    /* For Functions
        1. Invalid argument type
        2. Repeated variable name
    */
    return 0;
}

void firstPass(ASTNode* node, int struct_done)
{
    if (!node)
        return;

    if (node->sym_index == 57)
    {
        // <program> -> <funcList> <mainFunction>
        firstPass(node->children[0], 0);
        firstPass(node->children[1], 0);
        firstPass(node->children[0], 1);
        firstPass(node->children[1], 1);
    }
    else if (node->sym_index == 60 || node->sym_index == 58) //Function names parsed
    {
        // <function> -> <inputList><outputList> <stmts>
        
        if (!struct_done) 
        {
            printf("function name : %s\n", node->token->lexeme);

            TypeLog* mediator = getMediator(node->token->lexeme);
            mediator->refCount = 1;
            mediator->entry.entryType = FUNCTION;
            mediator->entry.width = -1;
            mediator->entry.index = funcCount++;

            FuncEntry* entry = calloc(1, sizeof(FuncEntry));
            mediator->entry.structure = entry;
            entry->argTypes = calloc(1, sizeof(TypeInfoList));
            entry->retTypes = calloc(1, sizeof(TypeInfoList));
        }
        
        firstPass(node->children[2], struct_done);
    }
    else if (node->sym_index == 68)
    {
        // <stmts> -> <definitions> <declarations> <funcBody> <return>
        firstPass(node->children[0], struct_done);
    }
    else if (node->sym_index == 71 && !struct_done && firstPassErrorCheck(node) != -1) //Type Definition Names Parsed
    {
        printf("%s %s \n", node->token->lexeme, node->children[0]->token->lexeme);
        trie_getRef(prefixTable, node->children[0]->token->lexeme)->entry.value =
            node->token->type;

        TypeLog* mediator = getMediator(node->children[0]->token->lexeme);
        mediator->refCount = 1;
        mediator->entry.entryType = DERIVED;
        mediator->entry.width = -1;
        mediator->entry.index = dataTypeCount++;
    }
    else if (node->sym_index == 108 && struct_done && firstPassErrorCheck(node) != -1) //Type Aliases Parsed
    {
        printf("typdef %s %s as %s\n", node->children[0]->token->lexeme, node->children[1]->token->lexeme, node->children[2]->token->lexeme);
        
        char* oldName = node->children[1]->token->lexeme;
        char* newName = node->children[2]->token->lexeme;

        TypeLog* mediator = getMediator(oldName);
        mediator->refCount++;

        trie_getRef(typeTable, newName)->entry.ptr = mediator;
        trie_getRef(prefixTable, newName)->entry.value = node->token->type;
        
    }
    
    firstPass(node->sibling, struct_done);
}


void printStructInfo(ASTNode* node)
{
    if (!node)
        return;

    if (node->sym_index == 57)
    {
        // <program> -> <funcList> <mainFunction>
        printStructInfo(node->children[0]);
        printStructInfo(node->children[1]);
    }
    else if (node->sym_index == 60 || node->sym_index == 58) //Function names parsed
    {
        // <function> -> <inputList><outputList> <stmts>
        printStructInfo(node->children[2]);
    }
    else if (node->sym_index == 68)
    {
        // <stmts> -> <definitions> <declarations> <funcBody> <return>
        printStructInfo(node->children[0]);
    }
    else if (node->sym_index == 71) //Type Definition Names Parsed
    {
        trie_getRef(prefixTable, node->children[0]->token->lexeme)->entry.value =
            node->token->type;

        TypeLog* mediator = getMediator(node->children[0]->token->lexeme);
        DerivedEntry* entry = mediator->entry.structure;

        printf("%s is Union: %d\n", entry->name, entry->isUnion);
        TypeInfoListNode* hd = entry->list->head;

        while (hd)
        {
            printf("\t%s is %s\n", hd->name, hd->type->entry.entryType == DERIVED ? ((DerivedEntry*)hd->type->entry.structure)->name : "BUILT IN");
            hd = hd->next;
        }
    }

    printStructInfo(node->sibling);
}

void secondPass(ASTNode* node, int** adj)
{
    if (!node)
        return;

    if (node->sym_index == 57)
    {
        // <program> -> <funcList> <mainFunction>
        secondPass(node->children[0], adj);
        secondPass(node->children[1], adj);
    }
    else if (node->sym_index == 60 || node->sym_index == 58) //Function Type Parsed
    {
        // <function> -> <inputList><outputList> <stmts>
        // Fill input argument 
        ASTNode* arg = node->children[0];

        TypeLog* mediator = getMediator(node->token->lexeme);
        FuncEntry* entry = mediator->entry.structure;
        
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
            entry->argTypes->tail->type = (arg->type->sibling ? getMediator(arg->type->sibling->token->lexeme) : getMediator(arg->type->token->lexeme));
            entry->argTypes->tail->type->refCount++;
            entry->argTypes->tail->name = calloc(arg->token->length + 1, sizeof(char));
            strcpy(entry->argTypes->tail->name, arg->token->lexeme);

            arg = arg->sibling;
        }

        ASTNode* ret = node->children[1];

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
            entry->retTypes->tail->type = (ret->type->sibling ? getMediator(ret->type->sibling->token->lexeme) : getMediator(ret->type->token->lexeme));
            entry->retTypes->tail->type->refCount++;
            entry->retTypes->tail->name = calloc(ret->token->length + 1, sizeof(char));
            strcpy(entry->retTypes->tail->name, ret->token->lexeme);

            ret = ret->sibling;
        }

        secondPass(node->children[2], adj);
    }
    else if (node->sym_index == 68)
    {
        // <stmts> -> <definitions> <declarations> <funcBody> <return>

        secondPass(node->children[0], adj);
        //TODO Parse declarations
    }
    else if (node->sym_index == 71 && secondPassErrorCheck(node) != -1) // Record/Union full type information parsed
    {
        // <typeDefinition> -> TK_RUID <fieldDefinitions>
        ASTNode* field = node->children[1];

        TypeLog* mediator = getMediator(node->children[0]->token->lexeme);
        
        mediator->entry.structure = calloc(1, sizeof(DerivedEntry));
        DerivedEntry* entry = mediator->entry.structure;
        entry->isUnion = node->token->type == TK_UNION;
        entry->name = calloc(node->children[0]->token->length + 1, sizeof(char));
        strcpy(entry->name, node->children[0]->token->lexeme);

        entry->list = calloc(1, sizeof(TypeInfoList));

        while (field)
        {
            if (!entry->list->head)
                entry->list->head = entry->list->tail = calloc(1, sizeof(TypeInfoListNode));
            else
            {
                entry->list->tail->next = calloc(1, sizeof(TypeInfoListNode));
                entry->list->tail = entry->list->tail->next;
            }

            TypeInfoListNode* infoNode = entry->list->tail;

            // TODO: Check error
            infoNode->type = getMediator(field->type->token->lexeme);
            infoNode->type->refCount++;
            infoNode->name = calloc(field->token->length + 1, sizeof(char));
            strcpy(infoNode->name, field->token->lexeme);

            field = field->sibling;
        }

    }

    secondPass(node->sibling, adj);
}

void loadSymbolTable(ASTNode *root)
{
    errList = calloc(1, sizeof(ErrorList));

    initTables();

    firstPass(root, 0);
    secondPass(root, NULL);
    printStructInfo(root);
    
}
