#include "Parser.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../helpers/Stack.h"
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



void computeNullable()
{
	int n = parserData->num_non_terminals + parserData->num_terminals;
	int** rules = parserData->productions;

	char** productionBitset = calloc(parserData->num_productions, sizeof(char*));
	for (int i = 0; i < parserData->num_productions; i++) {
		productionBitset[i] = calloc(BITNSLOTS(n), sizeof(char));


		for (int j = 1; j < parserData->productionSize[i]; j++)
			BITSET(productionBitset[i], rules[i][j]);


	}

	parserData->nullable = calloc(BITNSLOTS(n), sizeof(char));

	for (int i = 0; i < parserData->num_productions; i++) {
		int rhsSize = parserData->productionSize[i] - 1;
		int lhs = rules[i][0];

		if (rhsSize == 1 && !(rules[i][1]))
			BITSET(parserData->nullable, rules[i][0]);


	}
	int flag = 1;
	while (flag) {
		flag = 0;
		for (int i = 0; i < parserData->num_productions; i++) {
			int changed = 0;
			if (isEqual(bitAnd(parserData->nullable, productionBitset[i], n, &changed), productionBitset[i], n)) {
				BITSET(parserData->nullable, rules[i][0]);
				flag = changed;
			}
		}
	}
	free(productionBitset);
}

void printFollowSets()
{
	for (int i = 0; i < parserData->num_non_terminals; ++i)
	{
		printf("Follow set of %s: { ", parserData->symbolType2symbolStr[i + parserData->num_terminals]);
		for (int j = 0; j < parserData->num_terminals; ++j)
			if (BITTEST(parserData->followSet[i], j))
				printf("%s, ", parserData->symbolType2symbolStr[j]);
		printf("\b\b }\n");
	}
}

void populateFirstSets()
{
	int n = parserData->num_non_terminals;
	int tnt = n + parserData->num_terminals;

	parserData->firstSet = calloc(n, sizeof(char*));
	for (int i = 0; i < n; i++)
		parserData->firstSet[i] = calloc(parserData->num_terminals, sizeof(char));
	int** rules = parserData->productions;
	int flag = 1;
	char* nullable = parserData->nullable;

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
}

void populateFollowSets()
{
	int n = parserData->num_non_terminals;
	parserData->followSet = calloc(n, sizeof(char*));
	for (int i = 0; i < n; i++)
		parserData->followSet[i] = calloc(parserData->num_terminals, sizeof(char));
	int flag = 1;
	char* nullable = parserData->nullable;

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
					char* temp = calloc(parserData->num_terminals, sizeof(char));
					int nullableFlag = 1;
					for (int j = i + 1; j < k; j++)
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
							int dummy = 0;
							setUnion(temp, parserData->firstSet[rules[ind][j] - ts], nts, &dummy);
							if (!BITTEST(nullable, rules[ind][j]))
							{
								nullableFlag = 0;
								break;
							}
						}

					}
					setUnion(parserData->followSet[rules[ind][i] - ts], temp, nts, &change);
					if (nullableFlag || i == k - 1)
					{
						setUnion(parserData->followSet[rules[ind][i] - ts], parserData->followSet[lhs], nts, &change);
					}

				}
			}
		}
		flag = change;
	}
}

void populateSyncSets()
{
	for (int i = 0; i < parserData->num_non_terminals; ++i)
	{
		for (int j = 0; j < parserData->num_terminals; ++j)
		{
			if (parserData->parseTable[i][j] > -1)
				continue;

			if (BITTEST(parserData->followSet[i], j))
				parserData->parseTable[i][j] = -2;
		}
	}

	for (int i = 0; i < lexerData->num_keywords; ++i)
	{
		char* keyword = lexerData->keyword2Str[i];
		int index = trie_getVal(parserData->symbolStr2symbolType, keyword).value;
		assert(index > 0);

		for (int j = 0; j < parserData->num_non_terminals; ++j)
			if (parserData->parseTable[j][index] == -1)
				parserData->parseTable[j][index] = -2;
	}
}

void computeParseTable()
{
	parserData->parseTable = calloc(parserData->num_non_terminals, sizeof(int*));

	for (int i = 0; i < parserData->num_non_terminals; i++)
		parserData->parseTable[i] = calloc(parserData->num_terminals, sizeof(int));

	for (int i = 0; i < parserData->num_non_terminals; i++)
		for (int j = 0; j < parserData->num_terminals; j++)
			parserData->parseTable[i][j] = -1;

	int** rules = parserData->productions;
	char* nullable = parserData->nullable;

	for (int ind = 0; ind < parserData->num_productions; ind++)
	{
		int lhs = rules[ind][0] - parserData->num_terminals;
		int k = parserData->productionSize[ind];

		char* temp = calloc(parserData->num_terminals, sizeof(char));
		for (int j = 1; j < k; j++)
		{
			if (isTerminal(rules[ind][j]))
			{
				if (!BITTEST(temp, rules[ind][j]))
					BITSET(temp, rules[ind][j]);
				break;
			}

			int dummy = 0;
			setUnion(temp, parserData->firstSet[rules[ind][j] - parserData->num_terminals], parserData->num_non_terminals, &dummy);

			if (!BITTEST(nullable, rules[ind][j]))
				break;
		}
		
		if (!(rules[ind][1] == 0))
			for (int i = 0; i < parserData->num_terminals; i++)
				if (BITTEST(temp, i))
					parserData->parseTable[lhs][i] = ind;
		
		char* followSet = parserData->followSet[lhs];
		int ruleIsNullable = 1;
		for (int i = 0; i < parserData->num_terminals; i++)
			if (BITTEST(temp, i) && !BITTEST(nullable, i)) 
				ruleIsNullable = 0;
		
		if (ruleIsNullable || (rules[ind][1] == 0 && BITTEST(nullable, lhs + parserData->num_terminals)))
			for (int i = 0; i < parserData->num_terminals; i++)
				if (BITTEST(followSet, i))
					parserData->parseTable[lhs][i] = ind;

		free(temp);
	}
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
		fgets(BUFF, 200, fp);
		char* token = strtok(BUFF, " ");
		int count = 0;
		while (token)
		{
			if (strcmp(token, "\n") != 0 && strcmp(token, "\r\n") != 0)
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
}

void loadParser()
{
	assert(parserData == NULL);

	FILE* fp = fopen("./Parser/Parser_Structure.txt", "r");
	parserData = calloc(1, sizeof(ParserData));

	assert(fp != NULL);

	fscanf(fp, "%d %d %d %d\n", &parserData->num_terminals, &parserData->num_non_terminals, &parserData->num_productions, &parserData->start_index);
	parserData->symbolType2symbolStr = calloc(parserData->num_terminals + parserData->num_non_terminals, sizeof(char*));
	parserData->symbolStr2symbolType = calloc(1, sizeof(Trie));
	loadSymbols(fp);
	printf("Calculating Productions\n");
	loadProductions(fp);

	printf("Calculating Nullables\n");
	computeNullable();
	printf("Calculating First\n");
	populateFirstSets();
	printf("Calculating Follow\n");
	populateFollowSets();

	printf("Calculating PT\n");
	computeParseTable();
	populateSyncSets();

	fclose(fp);
}

int lexerToParserToken(int index)
{
	return trie_getVal(parserData->symbolStr2symbolType, lexerData->tokenType2tokenStr[index]).value;
}

void _pop(TreeNode** node, Stack* s)
{
	assert((*node)->symbol_index == top(s));

	pop(s);

	while ((*node)->parent_child_index == (*node)->parent->child_count - 1)
	{
		(*node) = (*node)->parent;
		if ((*node)->parent == NULL)
			return;
	}
	
	(*node) = (*node)->parent->children[(*node)->parent_child_index + 1];

	assert((*node)->symbol_index == top(s));
}

TreeNode* parseSourceCode(char* fileLoc)
{
	TreeNode* parseTree;
	FILE* fp = fopen(fileLoc, "r");
	loadFile(fp);

	Stack* s = calloc(1, sizeof(Stack));

	push(s, -1);
	push(s, parserData->start_index);

	parseTree = calloc(1, sizeof(TreeNode));
	parseTree->symbol_index = top(s);

	TreeNode* node = parseTree;

	Token* lookahead = getNextToken();

	while (lookahead != NULL)
	{
	//	printf("\nStack: \n\t");

		StackNode* temp = s->top;
		while (temp->data != -1)
		{
	//		printf("%s ", parserData->symbolType2symbolStr[temp->data]);
			temp = temp->prev;
		}

	//	printf("\nToken (%d): %s\n", lookahead->line_number, lexerData->tokenType2tokenStr[lookahead->type]);

		if (lookahead->type == TK_ERROR_LENGTH ||
			lookahead->type == TK_ERROR_PATTERN ||
			lookahead->type == TK_ERROR_SYMBOL)
		{
	//		printf("Inside Error token\n");
			lookahead = getNextToken();
			continue;
		}

		int stack_top = top(s);
		int input_terminal = lexerToParserToken(lookahead->type);;

		// if top of stack matches with input terminal (terminal at top of stack)
		if (stack_top == input_terminal)
		{
	//		printf("Stack top matches the input\n");
			_pop(&node, s);
			lookahead = getNextToken();
			continue;
		}

		// if top of stack is terminal but it is not matching with input look-ahead
		if (isTerminal(stack_top))
		{
	//		printf("Stack top doesn't match the lookahead\n");
			_pop(&node, s);
			continue;
		}

		// Here, top of stack is always non-terminal

		int production_number = parserData->parseTable[stack_top - parserData->num_terminals][input_terminal];

		// if it is a valid production
		if (production_number >= 0)
		{
			int* production = parserData->productions[production_number];
			int production_size = parserData->productionSize[production_number];

	//		printf("Expanding along the production\n\t%s -> ", parserData->symbolType2symbolStr[production[0]]);
			for (int i = 1; i < production_size; ++i);
	//			printf("%s ", parserData->symbolType2symbolStr[production[i]]);
	//		printf("\n");

			if (production_size == 2 && production[1] == 0)
			{
				_pop(&node, s);
				continue;
			}

			pop(s);
			node->child_count = production_size - 1;
			node->children = calloc(node->child_count, sizeof(TreeNode*));

			for (int i = production_size - 1; i > 0; --i)
			{
				push(s, production[i]);

				// -1 here because production[0] is the start symbol
				node->children[i - 1] = calloc(1, sizeof(TreeNode));

				node->children[i - 1]->parent = node;
				node->children[i - 1]->symbol_index = production[i];
				node->children[i - 1]->parent_child_index = i - 1;
				node->children[i - 1]->isLeaf = 0;
			}

			node = node->children[0];
			continue;
		}

		// if the production is not found and neither it is in sync set
		if (production_number == -1)
		{
	//		printf("No valid transistion found\n");
			lookahead = getNextToken();
			continue;
		}

		// left case is for sync set
		assert(production_number == -2);
	//	printf("Handling for the sync set");
		_pop(&node, s);
	}

	assert(top(s) == -1);
	
	fclose(fp);
	return parseTree;
}