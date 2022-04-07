#include "typeChecker.h"
#include <assert.h>

TypeLog* real, *integer, *boolean, *void_empty;

TypeLog* finalType(TypeLog* left, TypeLog* right, TokenType op)
{
    if (op == TK_ASSIGNOP)
    {
        if (left == right)
            return left;
        
        return NULL;
    }

    if (op == TK_PLUS || op == TK_MINUS || op == TK_MUL)
    {
        if (left == right)
            return left;
        
        return NULL;
    }

    if (op == TK_DIV)
    {
        if (left == real && right == real)
            return real;
        
        if (left == integer && right == integer)
            return real;

        return NULL;
    }

    if (op == TK_AND || op == TK_OR)
    {
        if (left == real && right == real)
            return real;
        
        if (left == integer && right == integer)
            return real;
        
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

    if (op == TK_EQ)
    {
        if (left == right)
            return left;
        
        return NULL;
    }

    assert(0);
}

void init()
{
    real = trie_getRef(globalSymbolTable, "real")->entry.ptr;
    integer = trie_getRef(globalSymbolTable, "int")->entry.ptr;
    boolean = trie_getRef(globalSymbolTable, "##bool")->entry.ptr;
    void_empty = trie_getRef(globalSymbolTable, "##void")->entry.ptr;
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
            checkAndAssignTypes(func->children[0]);
            func = func->sibling;
        }

        assignTypes(node->children[1]);
    }
    else if (node->sym_index == 58 || node->sym_index == 60)
    {
        // function/main-function
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

        node->derived_type = finalType(node->children[0]->derived_type, node->children[1]->derived_type, TK_ASSIGNOP);
    }
    else if (node->sym_index == 86)
    {
        // function call statement
        // TODO
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
        // TODO
    }
    else if (node->sym_index == 108)
    {
        // typedef
        node->derived_type = trie_getRef(globalSymbolTable, node->children[0]->token->lexeme);

        node->children[1]->type = node->children[2]->type = node->derived_type; 
    }
    else if (node->token->type == TK_PLUS || node->token->type == TK_MINUS || node->token->type == TK_MUL || node->token->type == TK_DIV)
    {
        assignTypes(node->children[0]);
        assignTypes(node->children[1]);

        node->derived_type = finalType(node->children[0]->derived_type, node->children[1]->derived_type, node->token->type);
    }
    else if (node->token->type == TK_ID)
    {
        
    }
}