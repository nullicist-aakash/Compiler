#pragma once
#include "ast.h"

// TODO: Remove this function in future
void removeComments(FILE* source, FILE* destination);
void printLexerOutput(FILE*, char*);
void printParseTree(FILE*, TreeNode*);
void printAST(FILE*, ASTNode*, int);
void printCompression(FILE*, TreeNode*, ASTNode*);

void printVariables(char* key, TrieEntry* entry);
void printFunctions(char* key, TrieEntry* entry);
void printActivationRecSize(char* key, TrieEntry* entry);
void printRecordDetails(char* key, TrieEntry* entry);