#pragma once

#include "parserDef.h"
/*rules
 isLeaf = true iff current node is leaf
 symbol_index = current non terminal index
 parent stores parent node and is NULL if current node is root
 token stores the corresponding token
 type != NULL iff current node stores the identifier
 children stores children
 sibling pointed to right element in a list
*/
typedef struct ASTNode
{
	int symbol_index;
	int isLeaf;
	struct ASTNode* parent;

	Token* token;
	ASTNode* type;
	int childCount;
	struct ASTNode** children;
	struct ASTNode* sibling;
} ASTNode;

ASTNode* createTree(TreeNode*);