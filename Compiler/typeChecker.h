#pragma once
#include "symbolTable.h"

void typeChecker_init(); 
void calculateOffsets(ASTNode*);
void assignTypes(ASTNode*);
extern int isTypeError;
