/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/

#pragma once
#define MAX 200
#include "trie.h"
#include "lexer.h"
#include "parserDef.h"
#include <stdio.h>

extern ParserData* parserData;
extern TreeNode* parseTree;
extern int ParseErr;

void loadParser();
void printParseTree(TreeNode* node);
TreeNode* parseInputSourceCode(char* fileLoc);
void freeParseTree(TreeNode*);
void ParseTreeDfs(TreeNode*, int*, int*);
