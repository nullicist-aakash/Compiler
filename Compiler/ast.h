/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/
#pragma once

#include "parserDef.h"
struct TypeLog;

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
	int isLeaf;
	int isGlobal;
	int sym_index;

	Token *token;
	struct ASTNode *type;
	struct TypeLog* derived_type;
	int childCount;
	struct ASTNode **children;
	struct ASTNode *sibling;
} ASTNode;

ASTNode *createAST(TreeNode *);
void printAST(ASTNode*, int);
void ASTDfs(ASTNode*, int*, int*);
