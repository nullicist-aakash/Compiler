/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/

#pragma once
#include "symbolTable.h"

extern int typeErr;

void typeChecker_init(); 
void calculateOffsets(ASTNode*);
void assignTypes(ASTNode*);
extern int isTypeError;
