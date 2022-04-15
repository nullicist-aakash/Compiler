/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/
#pragma once

#include "astDef.h"																														

struct TypeLog;

ASTNode *createAST(TreeNode *);
void printAST(ASTNode*, int);
void ASTDfs(ASTNode*, int*, int*);
