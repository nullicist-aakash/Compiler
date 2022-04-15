/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/
#pragma once

#include "parserDef.h"

typedef struct ASTNode
{
	int isLeaf;
	int isGlobal;
	int sym_index;

	Token* token;
	struct ASTNode* type;
	struct TypeLog* derived_type;
	int childCount;
	struct ASTNode** children;
	struct ASTNode* sibling;
} ASTNode;

struct TypeLog;

ASTNode *createAST(TreeNode *);
void freeAST(ASTNode*);