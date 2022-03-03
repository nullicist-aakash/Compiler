#ifndef PARSER_H
#define PARSER_H
#define MAX 200
#include "../helpers/Trie.h"
#include<stdio.h>
enum bool{ false,true };
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
	char** symbolType2symbolStr;
	int** parseTable;
	Trie* symbolStr2symbolType;
	Trie* lookupTable;	// move to global in future
} ParserData;
typedef struct TreeNode {
	int symbol;
	struct TreeNode** children;
}TreeNode;

typedef struct {
	char* lexeme;
	int numOfChildren;
	ParseTreeNode** children;
	ParseTreeNode* parent;
	int lineNum;
	enum bool isLeaf;

}ParseTreeNode;
extern ParserData* parserData;
void loadParser();

void parseSourceCode(char* fileLoc);

#endif