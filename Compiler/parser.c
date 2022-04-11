/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/

#include "parserDef.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "stack.h"
#include "logger.h"

ParserData* parserData;

int isTerminal(int index)
{
	return index >= 0 && index < parserData->num_terminals;
}

char* bitAnd(char* bitset3, char* bitset1, char* bitset2, int n, int* flag)
{
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
	char* bitset3 = calloc(n, sizeof(char));
	while (flag) {
		flag = 0;
		for (int i = 0; i < parserData->num_productions; i++) {
			int changed = 0;
			if (isEqual(bitAnd(bitset3,parserData->nullable, productionBitset[i], n, &changed), productionBitset[i], n)) {
				BITSET(parserData->nullable, rules[i][0]);
				flag = changed;
			}
		}
	}
	free(bitset3);
	for (int i = 0; i < parserData->num_productions; i++)
		free(productionBitset[i]);
	free(productionBitset);
}

void printFollowSets()
{
	for (int i = 0; i < parserData->num_non_terminals; ++i)
	{
		int cnt = 0;
		printf("Follow set of %s: { ", parserData->symbolType2symbolStr[i + parserData->num_terminals]);
		for (int j = 0; j < parserData->num_terminals; ++j)
			if (BITTEST(parserData->followSet[i], j)) {
				cnt++;
				printf("%s, ", parserData->symbolType2symbolStr[j]);
			}
		if(cnt)
			printf("\b\b");
		printf("}\n");
	}
}
void printFirstSets()
{
	for (int i = 0; i < parserData->num_non_terminals; ++i)
	{
		printf("First set of %s: { ", parserData->symbolType2symbolStr[i + parserData->num_terminals]);
		for (int j = 0; j < parserData->num_terminals; ++j)
			if (BITTEST(parserData->firstSet[i], j))
				printf("%s, ", parserData->symbolType2symbolStr[j]);
		printf("\b\b }\n");
	}
}
void addToFirstSet(int lhs, int symbol,int* change) {
	char** bitset1 = calloc(1,sizeof(char*));
	*bitset1 = parserData->firstSet[lhs];
	int t = parserData->num_terminals;
	int n = parserData->num_non_terminals;
	if (isTerminal(symbol)) {
		if (!BITTEST(*bitset1, symbol) && symbol) {
			BITSET(*bitset1, symbol);
			*change = 1;
		}
	}
	else 
		setUnion(*bitset1, parserData->firstSet[symbol - t], n, change);
	free(bitset1);
}
void addToFollowSet(char* bitset, int symbol, int* change, int* nullFlag) {
	char** bitset1 = calloc(1, sizeof(char*));
	*bitset1 = bitset;
	int t = parserData->num_terminals;
	int nt = parserData->num_non_terminals;
	char* nullable = parserData->nullable;
	if (isTerminal(symbol))
	{
		*nullFlag = 0;
		if (!BITTEST(*bitset1, symbol))
			BITSET(*bitset1, symbol);
	}
	else
	{
		setUnion(*bitset1, parserData->firstSet[symbol - t], nt, change);
		if (!BITTEST(nullable, symbol))
			*nullFlag = 0;
	}
	free(bitset1);
}
void populateFirstSets()
{
	int n = parserData->num_non_terminals;
	int t = parserData->num_terminals;
	int tnt = n + t;

	parserData->firstSet = calloc(n, sizeof(char*));
	for (int i = 0; i < n; i++)
		parserData->firstSet[i] = calloc(t, sizeof(char));
	int** rules = parserData->productions;
	int flag = 1;
	char* nullable = parserData->nullable;

	while (flag) {
		flag = 0;

		int change = 0;
		for (int i = 0; i < parserData->num_productions; i++) {
			int lhs = rules[i][0] - t;
			int k = parserData->productionSize[i];
			int j = 1;

			while (j < k) {
				addToFirstSet(lhs, rules[i][j], &change);
				if (!BITTEST(nullable, rules[i][j]))
					break;
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
					char* temp = calloc(ts, sizeof(char));
					int nullableFlag = 1;
					for (int j = i + 1; j < k; j++)
					{
						int dummy = 0;
						addToFollowSet(temp,rules[ind][j],&dummy,&nullableFlag);
						if (!nullableFlag)
							break;
					}
					setUnion(parserData->followSet[rules[ind][i] - ts], temp, nts, &change);
					if (nullableFlag || i == k - 1)
						setUnion(parserData->followSet[rules[ind][i] - ts], parserData->followSet[lhs], nts, &change);

					free(temp);
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

	FILE* fp = fopen(PARSER_DATA_LOC, "r");
	parserData = calloc(1, sizeof(ParserData));

	assert(fp != NULL);

	fscanf(fp, "%d %d %d %d\n", &parserData->num_terminals, &parserData->num_non_terminals, &parserData->num_productions, &parserData->start_index);
	parserData->symbolType2symbolStr = calloc(parserData->num_terminals + parserData->num_non_terminals, sizeof(char*));
	parserData->symbolStr2symbolType = calloc(1, sizeof(Trie));
	loadSymbols(fp);
	loadProductions(fp);

	computeNullable();
	populateFirstSets();
	populateFollowSets();
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

TreeNode* parseInputSourceCode(char* fileLoc)
{
	TreeNode* parseTree;
	FILE* fp = fopen(fileLoc, "r");

	if (fp == NULL)
	{
		printf("Error opening file: %s", fileLoc);
		exit(-1);
	}
	logIt("File open success: %s\n", fileLoc);

	loadFile(fp);

	Stack* s = calloc(1, sizeof(Stack));

	push(s, -1);
	push(s, parserData->start_index);

	parseTree = calloc(1, sizeof(TreeNode));
	parseTree->symbol_index = top(s);

	TreeNode* node = parseTree;

	Token* lookahead = getNextToken();

	int flag = 0;

	while (lookahead != NULL)
	{
		StackNode* temp = s->top;
		while (temp->data != -1)
			temp = temp->prev;

		if (lookahead->type == TK_ERROR_LENGTH ||
			lookahead->type == TK_ERROR_PATTERN ||
			lookahead->type == TK_ERROR_SYMBOL)
		{
			flag = 1; 

			if (lookahead->type == TK_ERROR_LENGTH)
				printf("Line %d \t\tError: Identifier is longer than the prescribed length.\n", lookahead->line_number);
			else if (lookahead->type == TK_ERROR_SYMBOL)
				printf("Line %d \t\tError: Unknwon Symbol <%s>\n", lookahead->line_number, lookahead->lexeme);
			else if (lookahead->type == TK_ERROR_PATTERN)
				printf("Line %d \t\tError: Unknown Pattern <%s>\n", lookahead->line_number, lookahead->lexeme);

			lookahead = getNextToken();
			continue;
		}
		
		int stack_top = top(s);
		int input_terminal = lexerToParserToken(lookahead->type);;


		// if top of stack matches with input terminal (terminal at top of stack)
		if (stack_top == input_terminal)
		{
			node->isLeaf = 1;
			node->token = lookahead;
			_pop(&node, s);
			lookahead = getNextToken();
			continue;
		}

		int line_number = lookahead->line_number;
		char* la_token = parserData->symbolType2symbolStr[input_terminal];
		char* lexeme = lookahead->lexeme;
		char* expected_token = parserData->symbolType2symbolStr[stack_top];

		// if top of stack is terminal but it is not matching with input look-ahead
		if (isTerminal(stack_top))
		{
			flag = 1;
			printf("Line %d \t\tError: The token %s for lexeme %s does not match with the expected token %s\n",line_number,la_token,lexeme,expected_token);
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
			node->productionNumber = production_number;

			// empty production
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
			}

			node = node->children[0];
			continue;
		}

		// if the production is not found and neither it is in sync set
		if (production_number == -1)
		{
			lookahead = getNextToken();
			continue;
		}

		// left case is for sync set
		assert(production_number == -2);

		flag = 1;
		printf("Line %d \t\tError: Invalid token %s encountered with value %s stack top %s\n", line_number, la_token, lexeme, expected_token);
		_pop(&node, s);
	}

	assert(top(s) == -1);
	free(s->top);
	free(s);
	fclose(fp);
	
	if (!flag)
		fprintf(stderr, "Input source code is syntactically correct.\n");

	return parseTree;
}

int getIntVal(char* str)
{
	int val = 0;

	for (int i = str[0] == '-' ? 1 : 0; i < strlen(str); ++i)
	{
		val *= 10;
		val += str[i] - '0';
	}

	return str[0] == '-' ? -val : val;
}

double getVal(Token* token)
{
	double zr = 0;
	if (token == NULL)
		return zr / zr;
	if (token->type != TK_RNUM && token->type != TK_NUM)
		return zr / zr;

	char* str = calloc(strlen(token->lexeme) + 1, sizeof(char));
	strcpy(str, token->lexeme);

	int dotLoc = -1;
	int eLoc = -1;

	for (int i = 0; i < strlen(str); ++i)
	{
		if (str[i] == '.')
			dotLoc = i;
		if (str[i] == 'E')
			eLoc = i;
	}

	if(dotLoc >=0)
		str[dotLoc] = '\0';
	
	if (eLoc >= 0)
		str[eLoc] = '\0';

	double val = getIntVal(str);

	if (dotLoc >= 0)
	{
		double f = getIntVal(str + dotLoc + 1);
		f /= 100;
		val += f;
	}

	if (eLoc >= 0)
	{
		double e = getIntVal(str + eLoc + 1);
		double pow = 1;
		for (int i = 0; i < (e > 0 ? e : -e); ++i)
			pow *= 10;

		if (e < 0)
			pow = 1 / pow;

		val *= pow;
	}

	free(str);
	return val;
}

void printParseTree(TreeNode* node, FILE* fptr)
{
	if (fptr == NULL) {
		perror("Error opening file");
		return;
	}
	if (node->parent != NULL)
	{
		char* A = node->isLeaf ? node->token->lexeme : "----";
		int B = node->isLeaf ? node->token->line_number : -1;
		char* C = !(node->isLeaf) ? "----" : parserData->symbolType2symbolStr[node->symbol_index];
		double D = getVal(node->token);
		char* E = node->parent == NULL ? "root" : parserData->symbolType2symbolStr[node->parent->symbol_index];
		char* F = node->isLeaf ? "yes" : "no";
		char* G = node->isLeaf ? "----" : parserData->symbolType2symbolStr[node->symbol_index];

		fprintf(fptr, "%30s %10d %30s %15f %30s %10s %30s\n", A, B, C, D, E, F, G);
	}
	else
		fprintf(fptr, "%30s %10d %30s %15s %30s %10s %30s\n", "----", -1, "----", "-nan", "ROOT", "no", "program");

	for (int i = 0; i < node->child_count; ++i)
		printParseTree(node->children[i], fptr);
}

void freeParseTree(TreeNode* node)
{
	for (int i = 0; i < node->child_count; ++i)
		freeParseTree(node->children[i]);

	if (node->token != NULL)
	{
		if (node->token->lexeme != NULL)
			free(node->token->lexeme);

		free(node->token);
	}

	if (node->child_count > 0)
		free(node->children);

	free(node);
}