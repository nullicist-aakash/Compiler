#include "Parser.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
	/* for CHAR_BIT */
#define CHAR_BIT 8
#define BITMASK(b) (1 << ((b) % CHAR_BIT))
#define BITSLOT(b) ((b) / CHAR_BIT)
#define BITSET(a, b) ((a)[BITSLOT(b)] |= BITMASK(b))
#define BITCLEAR(a, b) ((a)[BITSLOT(b)] &= ~BITMASK(b))
#define BITTEST(a, b) ((a)[BITSLOT(b)] & BITMASK(b))
#define BITNSLOTS(nb) ((nb + CHAR_BIT - 1) / CHAR_BIT)

ParserData* parserData;
void printBitset(char* bitset, int n) {
	printf("BITSET : ");
	for (int i = 0; i < n; i++) {
		if (BITTEST(bitset, i)) printf("%d ", i);
	}
	printf("\n");
}
int isTerminal(int index)
{
	return index >= 0 && index < parserData->num_terminals;
}
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
void setUnion(char* bitset1, char* bitset2, int n, int* flag) {


	for (int i = 0; i < n; i++) {
		char c = bitset1[i];
		
		bitset1[i] = bitset1[i] | bitset2[i];
		if (c != bitset1[i])  *flag = 1; 		

	}
}


char* getNullable()
{
	int n = parserData->num_non_terminals + parserData->num_terminals;
	int** rules = parserData->productions;

	char** productionBitset = calloc(parserData->num_productions, sizeof(char*));
	for (int i = 0; i < parserData->num_productions; i++) {
		productionBitset[i] = calloc(BITNSLOTS(n), sizeof(char));
		
		
		for (int j = 1; j < parserData->productionSize[i]; j++)
			BITSET(productionBitset[i], rules[i][j]);

		
	}

	char* nullable = calloc(BITNSLOTS(n), sizeof(char));

	for (int i = 0; i < parserData->num_productions; i++) {
		int rhsSize = parserData->productionSize[i] - 1;
		int lhs = rules[i][0];

		if (rhsSize == 1 && !(rules[i][1]))
			BITSET(nullable, rules[i][0]);
		
		
	}
	//printf("INITIALLY nullable : %s\n", nullable);
	int flag = 1;
	while (flag) {
		flag = 0;
		for (int i = 0; i < parserData->num_productions; i++) {
			int changed = 0;
			if (isEqual(bitAnd(nullable, productionBitset[i], n, &changed), productionBitset[i], n)) {
				BITSET(nullable, rules[i][0]);
				flag = changed;
			}
		}
	}
	printf("IN NULLABLE FUNC:\n");
	int cntt = 0;
	for (int i = 0; i < BITNSLOTS(n); i++) {
		printf("%d ", nullable[i]);
	}
	printf("\n");
	for (int i = 0; i < n; i++) {
		if (BITTEST(nullable, i))
			cntt += 1;
	};
	return nullable;
}
void getFirstSet()
{
	int n = parserData->num_non_terminals;
	int tnt = n + parserData->num_terminals;

	parserData->firstSet = calloc(n, sizeof(char*));
	for (int i = 0; i < n; i++)
		parserData->firstSet[i] = calloc(parserData->num_terminals, sizeof(char));
	int** rules = parserData->productions;
	int flag = 1;
	char* nullable = getNullable();

	while (flag) {
		flag = 0;

		int change = 0;
		for (int i = 0; i < parserData->num_productions; i++) {
			int lhs = rules[i][0] - parserData->num_terminals;
			printf("lhs:%d %d\n", lhs,parserData->num_terminals);

			int k = parserData->productionSize[i];
			int nullableUntilNow = 1;
			int j = 1;

			while (j < k && nullableUntilNow) {
				if (isTerminal(rules[i][j])) {
					nullableUntilNow = 0;
					if (rules[i][j] == 0) {
					}
					if (!BITTEST(parserData->firstSet[lhs], rules[i][j])) {
						if (rules[i][j]) {
							BITSET(parserData->firstSet[lhs], rules[i][j]);

							change = 1;
						}
						else
							continue;
					}
					j++;
					continue;
				}
				setUnion(parserData->firstSet[lhs], parserData->firstSet[rules[i][j] - parserData->num_terminals], n, &change);
				if (!BITTEST(nullable, rules[i][j]))
					nullableUntilNow = 0;
				j++;
			}

		}
		flag = change;
	}
}

void getFollowSet()
{
	int n = parserData->num_non_terminals;
	parserData->followSet = calloc(n, sizeof(char*));
	for (int i = 0; i < n; i++)
		parserData->followSet[i] = calloc(parserData->num_terminals, sizeof(char));
	int flag = 1;
	char* nullable = getNullable();
	
	while (flag) {
		flag = 0;
		int change = 0;
		int** rules = parserData->productions;
		for (int ind = 0; ind < parserData->num_productions; ind++) {
			int k = parserData->productionSize[ind];
			int lhs = rules[ind][0] - parserData->num_terminals;
			for (int i = k - 1; i > 0; i--) {
				setUnion(parserData->followSet[rules[ind][i] - parserData->num_terminals], parserData->followSet[lhs], n, &change);
				if (isTerminal(rules[ind][i]) && !(rules[ind][i]) && !BITTEST(nullable, rules[ind][i]))
					break;
			}
		}
		for (int ind = 0; ind < parserData->num_productions; ind++) {
			int k = parserData->productionSize[ind];
			int lhs = rules[ind][0];
			int i = 1, j;
			
			while (i < k-1) {
				j = i + 1;
				while (j < k) {
					if (isTerminal(rules[ind][j] && !(rules[ind][j]))) {
						i = j + 1;
						break;
					}
					if (BITTEST(nullable, rules[ind][j])) {
						for (int r = i + 1; r <= j; r++) {
							setUnion(parserData->followSet[rules[ind][r] - parserData->num_terminals], parserData->followSet[rules[r][j] - parserData->num_terminals], n, &change);
						}
						j++;
					}
					else {
						i = j + 1;
						break;
					}
				}
			}
		}
		flag = change;
	}
}

int** getParseTable()
{
	int** parseTable = calloc(parserData->num_non_terminals, sizeof(int*));
	for (int i = 0; i < parserData->num_non_terminals; i++) {
		parseTable[i] = calloc(parserData->num_terminals, sizeof(int));
	}
	int** rules = parserData->productions;
	char* nullable = getNullable();
	for (int ind = 0; ind < parserData->num_productions; ind++) {
		int lhs = rules[ind][0]- parserData->num_terminals;
		
		char* firstSet = parserData->firstSet[lhs];

		for (int i = 0; i < parserData->num_non_terminals; i++)
			if (BITTEST(firstSet, i)) {
				//printf("%d %d %d %d\n",lhs,i,ind);
				parseTable[lhs][i] = ind;
				//printf("%d %d\n", lhs);

			}
		

		char* followSet = parserData->followSet[lhs];
		if (nullable[lhs]) {

			for (int i = 0; i < parserData->num_terminals; i++) {
				printf("HELLO\n");
				for (int l = 0; l < parserData->num_non_terminals; l++)
				{
					printf("%d ", followSet[l]);
				}
				printf("\n");
				if (BITTEST(followSet, i)) {
					printf("%d %d %d\n", lhs, i, ind);
					parseTable[lhs][i] = ind;
					//printf("%d %d\n", lhs);

				}

			}
		}

	}
	return parseTable;
}
void loadSymbols(FILE* fp)
{
	for (int i = 0; i < parserData->num_terminals + parserData->num_non_terminals; ++i)
	{
		char BUFF[64];
		fscanf(fp, "%s\n", BUFF);
		parserData->symbolType2symbolStr[i] = calloc(strlen(BUFF) + 1, sizeof(char));
		strcpy(parserData->symbolType2symbolStr[i], BUFF);
		//printf("%d %s\n", i, BUFF);
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
		fgets(BUFF, 200, fp);
		char* token = strtok(BUFF, " ");
		int count = 0;
		while (token)
		{
			if(strcmp(token,"\n")==0 ){}
			else
			{
				symbols[count] = trie_getVal(parserData->symbolStr2symbolType, token).value;
				count++;
			}
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
	getFirstSet();
	//getFollowSet();
	//int** parseTable = getParseTable();
}



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
	parserData = calloc(1, sizeof(ParserData));

	assert(fp != NULL);

	fscanf(fp, "%d %d %d\n", &parserData->num_terminals, &parserData->num_non_terminals, &parserData->num_productions);
	printf("%d %d %d\n", parserData->num_non_terminals, parserData->num_terminals, parserData->num_productions);
	parserData->symbolType2symbolStr = calloc(parserData->num_terminals + parserData->num_non_terminals, sizeof(char*));
	parserData->symbolStr2symbolType = calloc(1, sizeof(Trie));
	loadSymbols(fp);
	loadProductions(fp);
	fclose(fp);
}

void parseSourceCode(char* fileLoc)
{

}
