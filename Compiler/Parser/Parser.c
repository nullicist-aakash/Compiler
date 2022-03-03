#include "Parser.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../Lexer/Lexer.h"

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
		if (c != bitset1[i])  *flag = *flag + 1; 		

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
	for (int i = 0; i < parserData->num_non_terminals; i++) {
		printf("First set of nonterminal %d -", i);
		printBitset(parserData->firstSet[i],parserData->num_terminals);
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
		int ts = parserData->num_terminals;
		int nts = parserData->num_non_terminals;
		int** rules = parserData->productions;
		for (int ind = 0; ind < parserData->num_productions; ind++) {
			int k = parserData->productionSize[ind];
			int lhs = rules[ind][0] - ts;
			for (int i = 1; i < k; i++) {
				if (!isTerminal(rules[ind][i]))
				{
					char * temp = calloc(parserData->num_terminals, sizeof(char));
					int nullableFlag = 1;
					for(int j = i + 1 ;j< k;j++)
					{
						if (isTerminal(rules[ind][j]))
						{
							nullableFlag = 0;
							if (!BITTEST(temp, rules[ind][j]))
							{
								BITSET(temp, rules[ind][j]);
							}
							break;
						}
						else
						{
							int dummy=0;
							setUnion(temp,parserData->firstSet[rules[ind][j]-ts],nts,&dummy);
							if (!BITTEST(nullable, rules[ind][j]))
							{
								nullableFlag = 0;
								break;
							}
						}

					}
					setUnion(parserData->followSet[rules[ind][i] - ts], temp, nts, &change);
					if (nullableFlag || i==k-1)
					{
						setUnion(parserData->followSet[rules[ind][i] - ts], parserData->followSet[lhs], nts, &change); 
					}

				}
			}
		}
		flag = change;
		
	}
	for (int i = 0; i < parserData->num_non_terminals; i++) {
		printf("Follow set for %d : ", i);
		printBitset(parserData->followSet[i], parserData->num_terminals);
	}
}


int** getParseTable()
{
	int** parseTable = calloc(parserData->num_non_terminals, sizeof(int*));
	for (int i = 0; i < parserData->num_non_terminals; i++) {
		parseTable[i] = calloc(parserData->num_terminals, sizeof(int));

	}
	for (int i = 0; i < parserData->num_non_terminals; i++)
	{

		for (int j = 0; j < parserData->num_terminals; j++)
		{
			parseTable[i][j]=-1;
		}
	}
	int** rules = parserData->productions;
	char* nullable = getNullable();
	for (int ind = 0; ind < parserData->num_productions; ind++) {
		int lhs = rules[ind][0] - parserData->num_terminals;
		int k = parserData->productionSize[ind];


		char* temp = calloc(parserData->num_terminals, sizeof(char));
		for (int j = 1; j < k; j++)
		{
			if (isTerminal(rules[ind][j]))
			{

				if (!BITTEST(temp, rules[ind][j]))
				{
					BITSET(temp, rules[ind][j]);
				}
				break;
			}
			else
			{
				int dummy = 0;
				setUnion(temp, parserData->firstSet[rules[ind][j] - parserData->num_terminals], parserData->num_non_terminals, &dummy);
				if (!BITTEST(nullable, rules[ind][j]))
				{
					break;
				}
			}

		}
		if (lhs == 2)
		{
			printf("=======================\n");
			printBitset(temp, parserData->num_terminals);
		}
		if (!(rules[ind][1] == 0))
		{
			for (int i = 0; i < parserData->num_terminals; i++)
			{
				if (BITTEST(temp, i)) {
					parseTable[lhs][i] = ind;
				}
			}
		}
		char* followSet = parserData->followSet[lhs];
		int ruleIsNullable = 1;
		for (int i = 0; i <parserData->num_terminals; i++) {
			if (BITTEST(temp,i) && !BITTEST(nullable, i)) ruleIsNullable = 0;
		}
		if (ruleIsNullable || (rules[ind][1]==0 && BITTEST(nullable,lhs+parserData->num_terminals))) {

			for (int i = 0; i < parserData->num_terminals; i++) {
				if (BITTEST(followSet, i)) {
					parseTable[lhs][i] = ind;
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
		printf("%d %s\n", i, BUFF);
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
	printf("First set computed..\n");
	getFollowSet();
	int** parseTable = getParseTable();

	for (int j = 0; j < parserData->num_terminals; j++)
	{
		printf("printing %d column\n", j + 1);
		for (int i = 0; i < parserData->num_non_terminals; i++)
		{
			printf("%d) ", i + 1);
			printf("%d ", parseTable[i][j] == -1 ? parseTable[i][j] : parseTable[i][j] + 1);
			printf("\n");
		}
		printf("---------------------------------------");
	}
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

//void parseSourceCode(char* fileLoc)
//{
//	FILE* fp = fopen(fileLoc, "r");
//
//	if (!fp)
//	{
//		fprintf(stderr, "Error opening file %s: %s\n", path, strerror(errno));
//		return;
//	}
//
//	loadFile(fp);
//
//	Token* tk;
//	while ((tk = getNextToken()) != NULL)
//	{
//		/*if (tk->type == TK_ERROR_LENGTH)
//			printf("Line no. %d: Error: Identifier length is greater than the prescribed length.\n", tk->line_number);
//		else if (tk->type == TK_ERROR_SYMBOL)
//			printf("Line no. %d: Error: Unknwon Symbol <%s>\n", tk->line_number, tk->lexeme);
//		else if (tk->type == TK_ERROR_PATTERN)
//			printf("Line no. %d: Error: Unknown Pattern <%s>\n", tk->line_number, tk->lexeme);
//		else
//			printf("Line no. %d\tLexeme %s\t\tToken %s\n", tk->line_number, tk->lexeme, lexerData->tokenType2tokenStr[tk->type]);*/
//	}
//}
