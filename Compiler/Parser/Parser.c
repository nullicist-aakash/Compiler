#include "Parser.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>		/* for CHAR_BIT */

#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

ParserData* parserData;

//int isTerminal(int index)
//{
//	return index >= 0 && index < parserData->num_terminals;
//}

//int isTerminal(char* str)
//{
//	int index = trie_getVal(parserData->symbolStr2symbolType, str).value;
//	return isTerminal(index);
//}
char* bitAnd(char* bitset1, char* bitset2, int n, int* flag)
{
	char* bitset3 = calloc(n, sizeof(char));
	for (int i = 0; i < BITNSLOTS(n); i++) {
		bitset3[i] = bitset1[i] & bitset2[i];
		if (bitset3[i] != bitset1[i])
			*flag = 1;
	}
	return bitset3;
}

int isEqual(char* bitset1, char* bitset2, int n) {
	for (int i = 0; i < n; ++i)
		if (bitset1[i] != bitset2[i])
			return 0;
	return 1;
}


char* getNullable()
{
	int n = parserData->num_non_terminals + parserData->num_terminals;
	char** productionBitset = calloc(parserData->num_productions, sizeof(char*));
	for (int i = 0; i < parserData->num_productions; i++) {
		productionBitset[i] = calloc(BITNSLOTS(n), sizeof(char));
		for (int j = 1; j < parserData->productionSize[i]; j++)
			BITSET(productionBitset[i], j);
	}

	char* nullable = calloc(BITNSLOTS(n), sizeof(char));
	int** rules = parserData->productions;

	for (int i = 0; i < parserData->num_productions; i++) {
		int rhsSize = parserData->productionSize[i] - 1;
		int lhs = rules[i][0];

		if (rhsSize == 1 && !(rules[i][1]))
			BITSET(nullable, rules[i][0]);
	}
	printf("INITIALLY nullable : %s\n", nullable);
	int flag = 1;
	while (flag) {
		flag = 0;
		for (int i = 0; i < parserData->num_productions; i++) {
			int changed = 0;
			if (isEqual(bitAnd(nullable, productionBitset[i], n, &changed), productionBitset[i], n)) {
				BITSET(nullable, rules[i][0]);
				flag += changed;
			}
		}
	}
	printf("nullable = %s\n", nullable);
	return nullable;
}

void loadSymbols(FILE* fp)
{
	for (int i = 0; i < parserData->num_terminals + parserData->num_non_terminals; ++i)
	{
		char BUFF[64];
		fscanf(fp, "%s\n", BUFF);
		parserData->symbolType2symbolStr[i] = calloc(strlen(BUFF) + 1, sizeof(char));
		strcpy(parserData->symbolType2symbolStr[i], BUFF);

		TrieNode* ref = trie_getRef(parserData->symbolStr2symbolType, BUFF);
		ref->entry.value = i;
	}
}

void loadProductions(FILE* fp)
{
	parserData->productions = calloc(parserData->num_productions, sizeof(int*));
	parserData->productionSize = calloc(parserData->num_productions, sizeof(int));
	for (int i = 0; i < parserData->num_productions; i++)
	{
		char BUFF[200];
		int symbols[200];
		fscanf(fp, "%[^\n]", BUFF);
		char* token = strtok(BUFF, " ");
		int count = 0;
		while (token)
		{
			symbols[count] = trie_getVal(parserData->symbolStr2symbolType, token).value;
			count++;
			token = strtok(NULL, " ");
		}
		parserData->productions[i] = calloc(count, sizeof(int));
		parserData->productionSize[i] = count;
		for (int j = 0; j < count; j++)
			parserData->productions[i][j] = symbols[j];
		
	}

	for (int i = 0; i < parserData->num_productions; i++)
	{
		for (int j = 0; j < parserData->productionSize[i]; j++)
		{
			printf("%d ", parserData->productions[i][j]);
		}
		printf("\n");
	}
	char* bruh = getNullable();
	printf("BRUH %s\n", bruh);
}

//char** getFirstSet()
//{
//
//}
//
//char** getFollowSet()
//{
//
//}
//
//char** getParseTable()
//{
//
//}

void loadParser()
{
	assert(parserData == NULL);

	FILE* fp = fopen("./Parser/Parser_Structure.txt", "r");
	parserData = calloc(1, sizeof(parserData));

	assert(fp != NULL);

	fscanf(fp, "%d %d %d\n", &parserData->num_non_terminals, &parserData->num_terminals, &parserData->num_productions);
	parserData->symbolType2symbolStr = calloc(parserData->num_terminals + parserData->num_non_terminals, sizeof(char*));
	parserData->symbolStr2symbolType = calloc(1, sizeof(Trie));
	loadSymbols(fp);
	loadProductions(fp);
	fclose(fp);
}

void parseSourceCode(char* fileLoc)
{

}
