/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/

#include "typeChecker.h"
#include "logger.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

TypeLog* real, * integer, * boolean, * void_empty;
int localWhileCount = 0;    // Number of while loops in the function
int localIdentifierCount = 0;
int curSize = 4;
int** localAssigned;        // Matrix representing if a variable has been assigned in a scope
Trie* localSymbolTable;
int typeErr = 0;

int areCompatible(ASTNode* leftNode, ASTNode* rightNode)
{
    TypeLog* left = leftNode->derived_type;
    TypeLog* right = rightNode->derived_type;
    while (leftNode && rightNode)
    {   
        left = leftNode->derived_type;
        right = rightNode->derived_type;
        

        if (left != right || left == boolean || left == void_empty || !left || !right)
            return 0;

        leftNode = leftNode->sibling;
        rightNode = rightNode->sibling;
    }
    return !leftNode && !rightNode;
}

int checkParameterLength(ASTNode* node, int input)
{
    ASTNode* paramList = node->children[input];
    TypeInfoListNode* cur;
    if(input==1)
        cur = ((FuncEntry*)((TypeLog*)trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr)->structure)->argTypes->head;
    else
        cur = ((FuncEntry*)((TypeLog*)trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr)->structure)->retTypes->head;
    while (cur && paramList)
    {
        cur = cur->next;
        paramList = paramList->sibling;
    }
    if (cur || paramList)
    {
        typeErr = 1;
        printf("ERROR : Line Number %d : Number of %s parameters does not match the formal parameters\n", node->token->line_number, input ? "input" : "output");
        return 0;
    }
    return 1;
}

int checkParameterType(ASTNode* node, int input)
{
    ASTNode* paramList = node->children[input];
    TypeInfoListNode* cur;
    if(input==1)
        cur = ((FuncEntry*)((TypeLog*)trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr)->structure)->argTypes->head;
    else
        cur = ((FuncEntry*)((TypeLog*)trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr)->structure)->retTypes->head;
    while (cur && cur->type)
    {
        if (cur->type != paramList->derived_type)
        {
            typeErr = 1;
            printf("ERROR : Line Number %d : Types of %s parameters does not match the formal parameters\n", node->token->line_number, input ? "input" : "output");
            return 0;
        }
        cur = cur->next;
        paramList = paramList->sibling;
    }
    return 1;
}

void recordAssignment(int index) {
    for (int i = 0; i <= localWhileCount; i++)
        localAssigned[i][index] = 1;
}

void reAllocate()
{
    if (curSize <= localWhileCount)
    {
        curSize *= 2;
        localAssigned = (int**)realloc(localAssigned, curSize*sizeof(int*));
        for (int i = curSize / 2; i < curSize; i++)
            localAssigned[i] = calloc(localIdentifierCount+identifierCount, sizeof(int));
    }
}

void getTypes(ASTNode* node, int* arr)
{
    //printf("Boolean childcount = %d\n", node->childCount);
    if (node->childCount == 0)
    {
        //printf("Searching for %s\n", node->token->lexeme);
        if (node->token->type == TK_RNUM)
            return;
        TypeLog* mediator = trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr;
        if(!mediator)
            mediator = trie_getRef(localSymbolTable, node->token->lexeme)->entry.ptr;
        if (!mediator)
        {
            assignTypes(node->sibling);
            return;
        }
        VariableEntry* varentry = mediator->structure;
        arr[mediator->index + (varentry->isGlobal ? localIdentifierCount : 0)] = 1;
    }
    else if (node->childCount == 1)
        getTypes(node->children[0], arr);
    else if (node->childCount == 2)
    {
        getTypes(node->children[0], arr);
        getTypes(node->children[1], arr);
    }
}

TypeLog* finalType(ASTNode* leftNode, ASTNode* rightNode, Token* opToken)
{
    TypeLog* left = leftNode->derived_type;
    TypeLog* right = rightNode ? rightNode->derived_type : NULL;
    TokenType op = opToken->type;

    if (!left)
        return NULL;
    if (op == TK_ASSIGNOP)
    {
        if (areCompatible(leftNode, rightNode))
            return void_empty;
        typeErr = 1;
        if (!rightNode ||!leftNode->derived_type || !rightNode->derived_type)
            return NULL;
        char* leftType = ((DerivedEntry*)left->structure)->name;
        char* rightType = ((DerivedEntry*)right->structure)->name;
        //logIt("Assignment with incompatible types at line no. %d \n", opToken->line_number);
        printf("ERROR : Line number %d : Assignment has a type mismatch as LHS variable is of type %s and RHS expression is of type %s\n",
            opToken->line_number, leftType, rightType);
        return NULL;
    }

    if (op == TK_PLUS || op == TK_MINUS)
    {
        if (left == right && left != boolean && left != void_empty && left && right)
            return right;

        typeErr = 1;
        if (!rightNode || !leftNode->derived_type || !rightNode->derived_type)
            return NULL;
        char* leftType = ((DerivedEntry*)left->structure)->name;
        char* rightType = ((DerivedEntry*)right->structure)->name;
        if (!leftType || !rightType)
            return NULL;
        //logIt("Operation %s %s %s with incompatible types at line no. %d \n", leftNode->token->lexeme, opToken->lexeme, rightNode->token->lexeme, opToken->line_number);
        printf("ERROR : Line number %d : Expression has a type mismatch, %s is of type %s and %s is of type %s\n",
            opToken->line_number, leftNode->token->lexeme, leftType, rightNode->token->lexeme, rightType);
        return NULL;
    }

    if (op == TK_MUL)
    {
        if (left == right && (left == real || left == integer) && left != boolean && left != void_empty)
            return left;

        // TODO:
        // scalar with integer allowed with record
        if (!rightNode || !right)
            return NULL;
        if (left->entryType == DERIVED && right == integer)
            return left;
        if (left == integer && right->entryType == DERIVED)
            return right;
            
        return NULL;
    }

    if (op == TK_DIV)
    {
        int first_type = left == real ? 0x01 : left == integer ? 0x02
            : 0x04;
        int second_type = right == real ? 0x01 : right == integer ? 0x02
            : 0x04;

        if ((first_type & 0x03) && (second_type & 0x03) && left != boolean && left != void_empty)
            return real;

        typeErr = 1;
        if (!rightNode || !leftNode->derived_type || !rightNode->derived_type)
            return NULL;
        char* leftType = ((DerivedEntry*)leftNode->derived_type->structure)->name;
        char* rightType = ((DerivedEntry*)rightNode->derived_type->structure)->name;
        if (!leftType || !rightType)
            return NULL;
        printf("ERROR : Line number %d : Incompatible types for operation %s with one argument of type %s and the other argument of type %s\n",
            opToken->line_number, opToken->lexeme, leftType, rightType);
        return NULL;
    }

    if (op == TK_AND || op == TK_OR)
    {
        if (left == boolean && right == boolean)
            return boolean;

        /*if(left && right)
            printf("ERROR : Line number %d : \n",
            opToken->line_number);*/
            
            
        typeErr = 1;
        if (!rightNode || !left || !right)
            return NULL;
        char* leftType = ((DerivedEntry*)left->structure)->name;
        char* rightType = ((DerivedEntry*)right->structure)->name;
        if (!leftType || !rightType)
            return NULL;
        //logIt("Operation %s %s %s with incompatible types at line no. %d \n", leftNode->token->lexeme, opToken->lexeme, rightNode->token->lexeme, opToken->line_number);
        printf("ERROR : Line number %d : Incompatible types for operation %s with one argument of type %s and the other argument of type %s\n",
            opToken->line_number, opToken->lexeme, leftType, rightType);
        return NULL;
    }

    if (op == TK_EQ || op == TK_NE || op == TK_GE || op == TK_LE || op == TK_LT || op == TK_GT)
    {
        if (left == real && right == real)
            return boolean;

        if (left == integer && right == integer)
            return boolean;

        typeErr = 1;
        if (!rightNode || !left || !right)
            return NULL;
        char* leftType = ((DerivedEntry*)left->structure)->name;
        char* rightType = ((DerivedEntry*)right->structure)->name;
        if (!leftType || !rightType)
            return NULL;
        //logIt("Operation %s %s %s with incompatible types at line no. %d \n", leftNode->token->lexeme, opToken->lexeme, rightNode->token->lexeme, opToken->line_number);4
        printf("ERROR : Line number %d : Incompatible types for operation %s with one argument of type %s and the other argument of type %s\n",
            opToken->line_number, opToken->lexeme, leftType, rightType);
        return NULL;
    }

    if (op == TK_NOT)
    {
        if (left == boolean)
            return boolean;

        typeErr = 1;
        if (!leftNode->derived_type)
            return NULL;
        char* leftType = ((DerivedEntry*)leftNode->derived_type->structure)->name;
        if (!leftType)
            return NULL;
        //logIt("Operation %s %s with incompatible types at line no. %d \n", leftNode->token->lexeme, opToken->lexeme, opToken->line_number);
        printf("ERROR : Line number %d : Incompatible types for operation %s with argument of type %s\n",
            opToken->line_number, opToken->lexeme, leftType);
        return NULL;
    }
    assert(0);
}

void typeChecker_init()
{
    real = trie_getRef(globalSymbolTable, "real")->entry.ptr;
    integer = trie_getRef(globalSymbolTable, "int")->entry.ptr;
    boolean = trie_getRef(globalSymbolTable, "##bool")->entry.ptr;
    void_empty = trie_getRef(globalSymbolTable, "##void")->entry.ptr;
    localSymbolTable = globalSymbolTable;
}

void assignTypes(ASTNode* node)
{
    if (!node)
        return;

    if (node->sym_index == -1){}
    else if (node->sym_index == 57)
    {
        // program -> functions, main
        assignTypes(node->children[0]);

        assignTypes(node->children[1]);
    }
    else if (node->sym_index == 58 || node->sym_index == 60)
    {
        // function/main-function
        localSymbolTable = ((FuncEntry*)((TypeLog*)trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr)->structure)->symbolTable;
        printf("%s\n",node->token->lexeme);
        //iterateTrie(localSymbolTable, printLocalTable);

        localWhileCount = 0;
        localIdentifierCount = ((FuncEntry*)((TypeLog*)trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr)->structure)->identifierCount;
        localAssigned = calloc(curSize, sizeof(int*));
        for (int i = 0; i < curSize; i++)
            localAssigned[i] = calloc(localIdentifierCount +identifierCount , sizeof(int));

        assignTypes(node->children[0]);
        assignTypes(node->children[1]);
        assignTypes(node->children[2]);

        int* reqTypes = calloc(localIdentifierCount+identifierCount, sizeof(int));
        ASTNode* cur = node->children[1];
        while (cur)
        {
            TypeLog *mediator = trie_getRef(localSymbolTable, cur->token->lexeme)->entry.ptr;
            reqTypes[mediator->index] = 1;
            cur = cur->sibling;
        }

        int flag = 1;
        for (int i = 0; i < localIdentifierCount+identifierCount; i++)
            if (reqTypes[i] && !localAssigned[0][i])
                flag = 0;
        free(reqTypes);
        if (!flag)
        {
            typeErr = 1;
            printf("ERROR : Line Number %d : Function output parameters not assigned a value\n", node->token->line_number);
        }

        for (int i = 0; i < curSize; i++)
            free(localAssigned[i]);
        free(localAssigned);
    }
    else if (node->sym_index == 68)
    {
        // stmts -> .. .. stmt ..
        assignTypes(node->children[1]);
        ASTNode* stmt = node->children[2];

        assignTypes(stmt);
    }
    else if (node->sym_index == 81)
    {
        // assignment --> <identifier> = <expression>
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);
        if(node->children[0]->token->type != TK_DOT)
        {
            TypeLog* mediator = trie_getRef(globalSymbolTable, node->children[0]->token->lexeme)->entry.ptr;
            if(!mediator)
                mediator = trie_getRef(localSymbolTable, node->children[0]->token->lexeme)->entry.ptr;
            if (!mediator)
            {
                assignTypes(node->sibling);
                return;
            }
            VariableEntry* varentry = mediator->structure;
            //printf("Updating %s with index %d\n", ((VariableEntry*)mediator->structure)->name, mediator->index + (node->children[0]->isGlobal ? identifierCount : 0));
            recordAssignment(mediator->index + (varentry->isGlobal ? localIdentifierCount : 0));
        }
        //else
        //{
        //    DerivedEntry* leftEntry = node->children[0]->derived_type->structure;
        //    // search for token on right of DOT

        //    TypeInfoListNode* field = leftEntry->list->head;
        //    while (field)
        //    {
        //        //printf("field->name = %s lexeme = %s\n", field->name, node->children[1]->token->lexeme);
        //        if (strcmp(field->name, node->children[1]->token->lexeme) == 0)
        //        {
        //            node->children[1]->derived_type = field->type;
        //            break;
        //        }
        //        recordAssignment()
        //        field = field->next;
        //    }
        //}
        // TODO: The type of an identifier of union data type is reported as an error.
        node->derived_type = finalType(node->children[0], node->children[1], node->token);
    }
    else if (node->sym_index == 86)
    {
        // function call statement
        // TODO:
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);
        //0 output
        //1 input
        if(checkParameterLength(node, 0))
            checkParameterType(node, 0);
        if(checkParameterLength(node, 1))
            checkParameterType(node, 1);
        ASTNode* cur = node->children[0];
        while (cur)
        {
            TypeLog* mediator = trie_getRef(globalSymbolTable, cur->token->lexeme)->entry.ptr;
            if(!mediator)
                mediator = trie_getRef(localSymbolTable, cur->token->lexeme)->entry.ptr;
            if (mediator)
            {
                VariableEntry* varentry = mediator->structure;
                //printf("Updating %s with index %d\n", ((VariableEntry*)mediator->structure)->name, mediator->index + node->children[0]->isGlobal ? identifierCount : 0);
                recordAssignment(mediator->index + (varentry->isGlobal ? localIdentifierCount : 0));
            }
            cur = cur->sibling;
        }

        node->derived_type = node->children[0]->derived_type;
    }
    else if (node->sym_index == 89)
    {
        // iterative statement, while

        localWhileCount++;
        reAllocate();
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);

        int* reqTypes = calloc(localIdentifierCount+identifierCount, sizeof(int));
        getTypes(node->children[0], reqTypes);

        int flag = 0;
        for (int i = 0; i < localIdentifierCount+identifierCount; i++)
        {
            if (reqTypes[i] && localAssigned[localWhileCount][i])
                flag = 1;
        }/*
        printf("ReqTypes - ==========\n");
        for (int i = 0; i < localIdentifierCount + identifierCount; i++)
            printf("%d ", reqTypes[i]);*/
        /*printf("\n");*/
        free(reqTypes);
        if (!flag)
        {
            typeErr = 1;
            printf("ERROR : Line Number %d : Variables of while loop are not assigned\n", node->token->line_number);
        }

        // for (int i = 0; i < curSize; i++)
        // {
        //     for (int j = 0; j < localIdentifierCount + identifierCount; j++)
        //         printf("%d ", localAssigned[i][j]);
        //     printf("\n");
        // }
        // printf("************ while end ***********\n");

        for(int i =0 ;i<localIdentifierCount + identifierCount;i++)
            localAssigned[localWhileCount][i]=0;
        localWhileCount--;
        node->derived_type = void_empty;
    }
    else if (node->sym_index == 90)
    {
        // if-else
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);
        assignTypes(node->children[2]);

        node->derived_type = void_empty;
    }
    else if (node->sym_index == 92)
    {
        // io
        assignTypes(node->children[0]);
        if (node->children[0]->token->type == TK_DOT)
        {
            node->derived_type = void_empty;
            assignTypes(node->sibling);
            return;
        }
        TypeLog* mediator = trie_getRef(globalSymbolTable, node->children[0]->token->lexeme)->entry.ptr;
        if (!mediator)
            mediator = trie_getRef(localSymbolTable, node->children[0]->token->lexeme)->entry.ptr;
        if (!mediator)
        {
            assignTypes(node->sibling);
            return;
        }
        //printf("mediator->index = %d\n", mediator->index);
        VariableEntry* varentry = mediator->structure;
        //printf("id count = %d\n", identifierCount);
        //printf("Updating %s with index %d\n", varentry->name, mediator->index + (varentry->isGlobal ? localIdentifierCount : 0));
        recordAssignment(mediator->index + (varentry->isGlobal ? localIdentifierCount : 0));

        node->derived_type = void_empty;
    }
    else if (node->sym_index == 63 || node->sym_index == 77 || node->sym_index==106)
    {
        // idList
        ASTNode* temp = node;

        while (temp)
        {
            TypeLog* mediator = trie_getRef(localSymbolTable, temp->token->lexeme)->entry.ptr;

            if (mediator == NULL)
                mediator = trie_getRef(globalSymbolTable, temp->token->lexeme)->entry.ptr;

            if (mediator)
            {
                VariableEntry* entry = mediator->structure;
                temp->derived_type = entry->type;
            }
            else
            {
                typeErr = 1;
                printf("ERROR : Line number %d : Variable %s is not declared\n", temp->token->line_number, temp->token->lexeme);
            }
            temp = temp->sibling;
        }
        // TODO:
    }
    else if (node->sym_index == 108)
    {
        // typedef
        node->derived_type = trie_getRef(globalSymbolTable, node->children[0]->token->lexeme)->entry.ptr;

        node->children[1]->derived_type = node->children[2]->derived_type = node->derived_type;
    }
    else if (
        node->token->type == TK_PLUS ||
        node->token->type == TK_MINUS ||
        node->token->type == TK_MUL ||
        node->token->type == TK_DIV ||
        node->token->type == TK_AND ||
        node->token->type == TK_OR ||
        node->token->type == TK_EQ ||
        node->token->type == TK_NE ||
        node->token->type == TK_GE ||
        node->token->type == TK_GT ||
        node->token->type == TK_LE ||
        node->token->type == TK_LT)
    {
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);

        /*printf("LINE NUMBER %d %d : ", node->token->line_number, node->token->type);
        printf("%d\n", node->sym_index);
        printf("%d\n", node->children[0]->derived_type->entryType);
        printf("%d\n", node->children[1]->derived_type->entryType);*/
        node->derived_type = finalType(node->children[0], node->children[1], node->token);
    }
    else if (node->token->type == TK_NOT)
    {
        assignTypes(node->children[0]);

        node->derived_type = finalType(node->children[0], NULL, node->token);
    }
    else if (node->token->type == TK_ID)
    {
        TypeLog* entry = trie_getRef(localSymbolTable, node->token->lexeme)->entry.ptr;

        if (entry == NULL)
            entry = trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr;

        if (entry)
        {
            node->derived_type = ((VariableEntry*)entry->structure)->type;
            //printf("Line number : %d, lexeme : %s, typeTag %d\n",node->token->line_number, node->token->lexeme,node->derived_type->entryType);
        }
        else
        {
            typeErr = 1;
            printf("ERROR : Line Number %d : Variable %s is not declared\n", node->token->line_number, node->token->lexeme);
        }
    }
    else if (node->token->type == TK_DOT)
    {
        // <dot> ===> <left> TK_DOT <right>
        assignTypes(node->children[0]);
        DerivedEntry* leftEntry = node->children[0]->derived_type->structure; 
        // search for token on right of DOT

        TypeInfoListNode* field = leftEntry->list->head;
        while (field)
        {
            //printf("field->name = %s lexeme = %s\n", field->name, node->children[1]->token->lexeme);
            if (strcmp(field->name, node->children[1]->token->lexeme) == 0)
            {
                node->children[1]->derived_type = field->type;
                break;
            }

            field = field->next;
        }
        

        if (node->children[1]->derived_type == NULL)
        {
            typeErr = 1;
            printf("ERROR : Line Number %d : Field %s does not exist\n", node->token->line_number, node->children[1]->token->lexeme);
            assignTypes(node->sibling);
            return;
        }
        node->derived_type = node->children[1]->derived_type;
    }
    else if (node->token->type == TK_NUM)
        node->derived_type = trie_getRef(globalSymbolTable, "int")->entry.ptr;
    else if (node->token->type == TK_RNUM)
        node->derived_type = trie_getRef(globalSymbolTable, "real")->entry.ptr;
    assignTypes(node->sibling);
}