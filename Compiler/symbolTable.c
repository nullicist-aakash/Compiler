#include "symbolTable.h"
#include "Trie.h"
#include <string.h>

typedef enum {
    NAME_REDEFINED,
    NON_EXISTENT_TYPE,
    NAME_TYPE_MISMTACH,
    REDEFINED_ALIAS,
    UNDEFINED_TYPE,
    INVALID_TYPE,
    DUPLICATE_FIELD_NAME,
}ErrorType;

typedef struct {
    ErrorType etype;
    ASTNode* errnode;
    ErrorListNode* next;
}ErrorListNode;

typedef struct {
    ErrorListNode* head;
}ErrorList;

typedef struct 
{
    ASTNode* node;
    ASTNodeListNode* next;

}ASTNodeListNode;

typedef struct
{
    ASTNodeListNode* head;
    ASTNodeListNode* tail;
}ASTNodeList;

Trie *typeTable;
Trie* prefixTable;
char errMsg[30];
ErrorList* errList;

void initTypeTable()
{
    typeTable = calloc(1, sizeof(Trie));

    TypeLog *intInfo = calloc(1, sizeof(TypeLog));
    intInfo->refCount = 1;
    intInfo->entry.entryType = INT;
    intInfo->entry.width = 4;

    trie_getRef(typeTable, "int")->entry.ptr = intInfo;

    TypeLog *realInfo = calloc(1, sizeof(TypeLog *));
    realInfo->refCount = 1;
    realInfo->entry.entryType = REAL;
    realInfo->entry.width = 4;

    trie_getRef(typeTable, "real")->entry.ptr = realInfo;
}

void initPrefixTable()
{
    prefixTable = calloc(1, sizeof(Trie));
}

TypeLog* getMediator(char* key) 
{
    TrieNode* node = trie_getRef(typeTable, key);

    if (node->entry.value == 0)
        node->entry.ptr = calloc(1, sizeof(TypeLog));

    return (TypeLog*)node->entry.ptr;
}

void addToList(ASTNodeList* list, ASTNode* val)
{
    // The beauty
    list->tail = (list->head ? list->tail->next : list->head) = calloc(1, sizeof(ASTNodeListNode));
    
    // The beast
    list->tail->node = value;
}

void addToErrorList(ASTNode* node, ErrorType type) {
    // The beauty
    list->tail = (list->head ? list->tail->next : list->head) = calloc(1, sizeof(ErrorListNode));

    // The beast
    list->tail->errnode = node;
    list->tail->errorType = type;
}

void printErrors() 
{

}

void firstPassErrorCheck(ASTNode* node) 
{
    if (node->token->type != TK_DEFINETYPE)
    {
        if (trie_exists(prefixTable, node->children[0]->token->lexeme))
            addToErrorList(node, NAME_REDEFINED);
        return;
    }

    // TODO : Errors 

}

void firstPassPreprocess(ASTNode* root, ASTNodeList* typeDefList, ASTNodeList* definitionList)
{
    // Step 1 : Segregate typdef and struct from AST 
    ASTNode* func = root->children[0];
    ASTNode* main = root->children[1];
    
    while (func != NULL) 
    {
        ASTNode* stmt = func->children[2]->children[0];

        while (stmt != NULL) {
            if (stmt->token->type == TK_DEFINETYPE)
                addtoList(typeDefList, stmt);
            else
                addToList(definitionList, stmt);
            stmt = stmt->sibling;
        }

        func = func->sibling;
    }

    func = root->children[1];

    ASTNode* stmt = func->children[2]->children[0];

    while (stmt != NULL) {
        if (stmt->token->type == TK_DEFINETYPE)
            addtoList(typeDefList, stmt);
        else
            addToList(definitionList, stmt);
        stmt = stmt->sibling;
    }
}

void firstPassPostprocess(ASTNodeList* typeDefList, ASTNodeList* definitionList) 
{
    // Struct definitions
    ASTNodeListNode* curr = definitionList->head;

    while (curr) 
    {
        if (firstPassErrorCheck(curr->node) == -1)
        {
            curr = curr->next;
            continue;
        }

        trie_getRef(prefixTable, curr->node->children[0]->token->lexeme)->entry.value =
            curr->node->token->type;

        TypeLog* mediator = getMediator(curr->node->children[0]->token->lexeme);
        mediator->refCount = 1;
        mediator->entry.type = DERIVED;
        mediator->entry.width = -1;

        curr = curr->next;
    }
    // Typedef 
    curr = typeDefList->head;
    while (curr)
    {
        if (firstPassErrorCheck(curr->node) == -1)
        {
            curr = curr->next;
            continue;
        }

        char* oldName = curr->node->children[1]->token->lexeme;
        char* newName = curr->node->children[2]->token->lexeme;

        TypeLog* mediator = getMediator(oldName);
        mediator->refCount++;

        trie_getRef(typeTable, newName)->entry.ptr = mediator;

        trie_getRef(prefixTable, newName)->entry.value = curr->node->token->type;

        curr = curr->next;
    }
    //printError
    printErrors();
}

void secondPass(ASTNodeList* definitionList)
{

}

void loadSymbolTable(ASTNode *root)
{
    errList = calloc(1, sizeof(ErrorList));
    ASTNodeList* typeDefList = calloc(1, sizeof(ASTNodeList));
    ASTNodeList* definitionList = calloc(1, sizeof(ASTNodeList));
    initTypeTable();
    initPrefixTable();

    firstPassPreprocess(root, typeDefList, definitionList);
    firstPassPostprocess(typeDefList, definitionList);
    secondPass(definitionList);
    
}