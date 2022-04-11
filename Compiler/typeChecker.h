#pragma once
#include "symbolTable.h"

void typeChecker_init();
void assignTypes(ASTNode*);
extern int isTypeError;
