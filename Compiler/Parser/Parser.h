#ifndef PARSER_H
#define PARSER_H
#define MAX 200
#include "../helpers/Trie.h"
#include "../Lexer/Lexer.h"
#include <stdio.h>

typedef struct 
{
	int num_non_terminals;
	int num_terminals;
	int num_productions;
	int start_index;

	int** productions;
	int* productionSize;
	char** firstSet;
	char** followSet;
	char* nullable;
	char** symbolType2symbolStr;
	int** parseTable;
	Trie* symbolStr2symbolType;
	Trie* lookupTable;	// move to global in future
} ParserData;

typedef struct TreeNode
{
	int symbol_index;
	int child_count;
	int parent_child_index;

	Token* token;

	int isLeaf;
	struct TreeNode* parent;
	struct TreeNode** children;
} TreeNode;

extern ParserData* parserData;
extern TreeNode* parseTree;

void loadParser();
TreeNode* parseSourceCode(char* fileLoc);

#endif