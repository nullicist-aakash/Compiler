#include "typeChecker.h"
#include <assert.h>
#include <string.h>

TypeLog* real, *integer, *boolean, *void_empty;

TypeLog* finalType(TypeLog* left, TypeLog* right, TokenType op)
{
    if (op == TK_ASSIGNOP)
    {
        if (left == right && left != boolean && left != void_empty)
            return left;
        
        return NULL;
    }

    if (op == TK_PLUS || op == TK_MINUS)
    {
        if (left == right && left!=boolean && left!=void_empty)
            return left;
        
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
        int first_type = left == real ? 0x01 : left == integer ? 0x02 : 0x04;
        int second_type = right == real ? 0x01 : right == integer ? 0x02 : 0x04;

        if ((first_type & 0x03) && (second_type & 0x03) && left != boolean && left != void_empty)
            return real;

        return NULL;
    }

    if (op == TK_AND || op == TK_OR)
    {
        if (left == boolean && right == boolean)
            return boolean;
        
        return NULL;
    }

    if (op == TK_EQ || op == TK_NE || op == TK_GE || op == TK_LE || op == TK_LT || op == TK_GT)
    {
        if (left == real && right == real)
            return boolean;
        
        if (left == integer && right == integer)
            return boolean;
        
        return NULL;
    }

    if (op == TK_NOT)
    {
        if (left == boolean)
            return boolean;
        return NULL;
    }
    assert(0);
}


Trie* localSymbolTable;
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

    if (node->sym_index == 57)
    {
        // program -> functions, main
        ASTNode* func = node->children[0];
        while (func)
        {
            assignTypes(func->children[0]);
            func = func->sibling;
        }

        assignTypes(node->children[1]);
    }
    else if (node->sym_index == 58 || node->sym_index == 60)
    {
        // function/main-function
        localSymbolTable = ((FuncEntry*)trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr)->symbolTable;
        assignTypes(node->children[2]);
    }
    else if (node->sym_index == 68)
    {
        // stmts -> .. .. stmt ..
        ASTNode* stmt = node->children[2];
        while (stmt)
        {
            assignTypes(stmt);
            stmt = stmt->sibling;
        }
    }
    else if (node->sym_index == 81)
    {
        // assignment --> <identifier> = <expression>
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);
        // TODO The type of an identifier of union data type is reported as an error.
        node->derived_type = finalType(node->children[0]->derived_type, node->children[1]->derived_type, TK_ASSIGNOP);
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
    else if (node->sym_index == 106)
    {
        // idList
        ASTNode* temp = node;
        
        while (temp)
        {
            temp->derived_type = ((VariableEntry *)trie_getRef(localSymbolTable, node->token->lexeme)->entry.ptr)->type;

            if (temp->derived_type == NULL)
                temp->derived_type = ((VariableEntry *)trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr)->type;
                
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

        node->derived_type = finalType(node->children[0]->derived_type, node->children[1]->derived_type, node->token->type);
    }
    else if (node->token->type == TK_NOT)
    {
        assignTypes(node->children[0]);

        node->derived_type = finalType(node->children[0]->derived_type, NULL, node->token->type);
    }
    else if (node->token->type == TK_ID)
    {
        node->derived_type = ((VariableEntry*)trie_getRef(localSymbolTable, node->token->lexeme)->entry.ptr)->type;

        if (node->derived_type == NULL)
            node->derived_type = ((VariableEntry*)trie_getRef(globalSymbolTable, node->token->lexeme)->entry.ptr)->type;
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
}