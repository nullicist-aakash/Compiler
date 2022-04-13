#include "typeChecker.h"
#include "logger.h"

#include <assert.h>
#include <string.h>

TypeLog *real, *integer, *boolean, *void_empty;
int isTypeError = 0;

int areCompatible(ASTNode *leftNode, ASTNode *rightNode)
{
    TypeLog *left = leftNode->derived_type;
    TypeLog *right = rightNode->derived_type;
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

TypeLog *finalType(ASTNode *leftNode, ASTNode *rightNode, Token *opToken)
{
    TypeLog *left = leftNode->derived_type;
    TypeLog *right = rightNode ? rightNode->derived_type : NULL;
    TokenType op = opToken->type;

    if (op == TK_ASSIGNOP)
    {
        if (areCompatible(leftNode, rightNode))
            return void_empty;

        isTypeError = 1;
        logIt("Assignment with incompatible types at line no. %d \n", opToken->line_number);
        return NULL;
    }

    if (op == TK_PLUS || op == TK_MINUS)
    {
        if (left == right && left != boolean && left != void_empty && left && right)
            return right;

        isTypeError = 1;
        logIt("Operation %s %s %s with incompatible types at line no. %d \n", leftNode->token->lexeme, opToken->lexeme, rightNode->token->lexeme, opToken->line_number);
        return NULL;
    }

    if (op == TK_MUL)
    {
        if (left == right && (left == real || left == integer) && left != boolean && left != void_empty)
            return left;

        // TODO

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

        return NULL;
    }

    if (op == TK_AND || op == TK_OR)
    {
        if (left == boolean && right == boolean)
            return boolean;

        isTypeError = 1;
        logIt("Operation %s %s %s with incompatible types at line no. %d \n", leftNode->token->lexeme, opToken->lexeme, rightNode->token->lexeme, opToken->line_number);
        return NULL;
    }

    if (op == TK_EQ || op == TK_NE || op == TK_GE || op == TK_LE || op == TK_LT || op == TK_GT)
    {
        if (left == real && right == real)
            return boolean;

        if (left == integer && right == integer)
            return boolean;

        isTypeError = 1;
        logIt("Operation %s %s %s with incompatible types at line no. %d \n", leftNode->token->lexeme, opToken->lexeme, rightNode->token->lexeme, opToken->line_number);
        return NULL;
    }

    if (op == TK_NOT)
    {
        if (left == boolean)
            return boolean;

        isTypeError = 1;
        logIt("Operation %s %s with incompatible types at line no. %d \n", leftNode->token->lexeme, opToken->lexeme, opToken->line_number);
        return NULL;
    }
    assert(0);
}

Trie *localSymbolTable;
void typeChecker_init()
{
    real = trie_getRef(globalSymbolTable, "real")->entry.ptr;
    integer = trie_getRef(globalSymbolTable, "int")->entry.ptr;
    boolean = trie_getRef(globalSymbolTable, "##bool")->entry.ptr;
    void_empty = trie_getRef(globalSymbolTable, "##void")->entry.ptr;
    localSymbolTable = globalSymbolTable;
}

void assignTypes(ASTNode *node)
{
    if (!node)
        return;

    if (node->sym_index == 57)
    {
        // program -> functions, main
        assignTypes(node->children[0]);

        assignTypes(node->children[1]);
    }
    else if (node->sym_index == 58 || node->sym_index == 60)
    {
        // function/main-function
        localSymbolTable = ((FuncEntry *)((TypeLog *)trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr)->structure)->symbolTable;
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);
        assignTypes(node->children[2]);
    }
    else if (node->sym_index == 68)
    {
        // stmts -> .. .. stmt ..
        assignTypes(node->children[1]);
        ASTNode *stmt = node->children[2];

        assignTypes(stmt);
    }
    else if (node->sym_index == 81)
    {
        // assignment --> <identifier> = <expression>
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);
        // TODO The type of an identifier of union data type is reported as an error.
        node->derived_type = finalType(node->children[0], node->children[1], node->token);
    }
    else if (node->sym_index == 86)
    {
        // function call statement
        // TODO
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);

        node->derived_type = node->children[0]->derived_type;
    }
    else if (node->sym_index == 89)
    {
        // iterative statement, while
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);

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

        node->derived_type = void_empty;
    }
    else if (node->sym_index == 63 || node->sym_index == 77)
    {
        // idList
        ASTNode *temp = node;

        while (temp)
        {
            TypeLog *mediator = trie_getRef(localSymbolTable, node->token->lexeme)->entry.ptr;

            if (mediator == NULL)
                mediator = trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr;

            VariableEntry *entry = mediator->structure;

            temp->derived_type = entry->type;
            temp = temp->sibling;
        }
        // TODO
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

        node->derived_type = finalType(node->children[0], node->children[1], node->token);
    }
    else if (node->token->type == TK_NOT)
    {
        assignTypes(node->children[0]);

        node->derived_type = finalType(node->children[0], NULL, node->token);
    }
    else if (node->token->type == TK_ID)
    {
        TypeLog *entry = trie_getRef(localSymbolTable, node->token->lexeme)->entry.ptr;

        if (entry == NULL)
            entry = trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr;

        node->derived_type = ((VariableEntry *)entry->structure)->type;
    }
    else if (node->token->type == TK_DOT)
    {
        // <dot> ===> <left> TK_DOT <right>
        assignTypes(node->children[0]);

        DerivedEntry *leftEntry = node->children[0]->derived_type->structure; //

        // search for token on right of DOT

        TypeInfoListNode *field = leftEntry->list->head;

        while (field)
        {
            if (strcmp(field->name, node->children[1]->token->lexeme) == 0)
            {
                node->children[1]->derived_type = field->type;
                break;
            }

            field = field->next;
        }

        assert(node->children[1]->derived_type != NULL);
        node->derived_type = node->children[1]->derived_type;
    }
    else if (node->token->type == TK_NUM)
        node->derived_type = trie_getRef(globalSymbolTable, "int")->entry.ptr;
    else if (node->token->type == TK_RNUM)
        node->derived_type = trie_getRef(globalSymbolTable, "real")->entry.ptr;
    assignTypes(node->sibling);
}