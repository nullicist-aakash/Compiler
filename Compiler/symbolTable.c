#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "symbolTable.h"
#include "Trie.h"

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

Trie *typeTable;
Trie* prefixTable;
int dataTypeCount = 0;
int typeDefCount = 0;
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
    else if (node->sym_index == 60 || node->sym_index == 58)
    {
        // <function> -> <inputList><outputList> <stmts>
        firstPass(node->children[2], struct_done);
    }
    else if (node->sym_index == 68)
    {
        // <stmts> -> <definitions> <declarations> <funcBody> <return>
        firstPass(node->children[0], struct_done);
    }
    else if (node->sym_index == 71 && !struct_done && firstPassErrorCheck(node) != -1)
    {
        printf("%s %s \n", node->token->lexeme, node->children[0]->token->lexeme);
        trie_getRef(prefixTable, node->children[0]->token->lexeme)->entry.value =
            node->token->type;

        TypeLog* mediator = getMediator(node->children[0]->token->lexeme);
        mediator->refCount = 1;
        mediator->entry.entryType = DERIVED;
        mediator->entry.width = -1;

        dataTypeCount++;
    }
    else if (node->sym_index == 108 && struct_done && firstPassErrorCheck(node) != -1)
    {
        printf("typdef %s %s as %s\n", node->children[0]->token->lexeme, node->children[1]->token->lexeme, node->children[2]->token->lexeme);
        
        char* oldName = node->children[1]->token->lexeme;
        char* newName = node->children[2]->token->lexeme;

        TypeLog* mediator = getMediator(oldName);
        mediator->refCount++;

        trie_getRef(typeTable, newName)->entry.ptr = mediator;
        trie_getRef(prefixTable, newName)->entry.value = node->token->type;
        
        typeDefCount++;
    }
    
    firstPass(node->sibling, struct_done);
}

void loadSymbolTable(ASTNode *root)
{
    errList = calloc(1, sizeof(ErrorList));

    initTypeTable();
    initPrefixTable();

    firstPass(root,0);
    
}
