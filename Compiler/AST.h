#pragma once

#include "parserDef.h"

typedef struct ASTNode
{
	int symbol_index;
	int isLeaf;
	struct ASTNode* parent;

	Token* token;
	int childCount;
	struct ASTNode** children;
	struct ASTNode* sibling;
} ASTNode;

ASTNode* createTree(TreeNode*);