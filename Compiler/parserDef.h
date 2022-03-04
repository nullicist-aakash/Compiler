/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  NIHIR AGARWAL			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/
#include "Trie.h"
#include "Stack.h"
#include <stdio.h>
#include "Lexer.h"


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

#define PARSER_H
#define MAX 200

/* for CHAR_BIT */
#define CHAR_BIT 8
#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)
#define PARSER_DATA_LOC "Parser_Structure.txt"

ParserData* parserData;