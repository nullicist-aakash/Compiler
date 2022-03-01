#ifndef PARSER_H
#define PARSER_H

#include "../helpers/Trie.h"
#include<stdio.h>
typedef struct 
{
	int num_non_terminals;
	int num_terminals;
	int num_productions;
	
	int** productions;
	int* productionSize;
	char** firstSet;
	char** followSet;
	char** symbolType2symbolStr;
	Trie* symbolStr2symbolType;
	Trie* lookupTable;	// move to global in future
} ParserData;

extern ParserData* parserData;
void loadParser();

void parseSourceCode(char* fileLoc);

#endif