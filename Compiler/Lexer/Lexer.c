#include "Lexer.h"
#include "../helpers/Trie.h"
#include <errno.h>
#include <assert.h>

#define TWIN_BUFF_SIZE 50
#define DEFINITION_LOC "./DFA_Structure.txt"

typedef struct Buffer
{
	FILE* fp;
	char* working;
	char* archived;

	int charTaken;
} Buffer;

int loaded = 0;

int num_tokens, num_states, num_transitions, num_finalstates, num_keywords;
Buffer* b = NULL;
int** transitions;
TokenType* finalStates;
char** tokenType2tokenStr;
Trie* tokenStr2tokenType;
Trie* symbolTable;	// move to global in future

char getChar(int i)
{
	if (b->charTaken == i)
	{
		char* temp = b->archived;
		b->archived = b->working;
		b->working = temp;

		char c = 0;
		int reads = 0;
		while (fscanf(b->fp, "%c", c) != EOF && reads != TWIN_BUFF_SIZE)
			b->working[reads++] = c;

		if (reads != TWIN_BUFF_SIZE)
			b->working[reads] = '\0';

		b->charTaken += TWIN_BUFF_SIZE;
	}

	if (b->charTaken - i <= TWIN_BUFF_SIZE)
	{
		i %= TWIN_BUFF_SIZE;
		return b->working[i];
	}

	i %= TWIN_BUFF_SIZE;
	return b->archived[i];
}

void loadTokens(FILE* fp)
{
	if (tokenStr2tokenType == NULL)
		tokenType2tokenStr = calloc(num_tokens, sizeof(char*));
	
	for (int i = 0; i < num_tokens; ++i)
	{
		fscanf(fp, "%s\n", tokenType2tokenStr[i]);

		TrieNode* ref = trie_getRef(tokenStr2tokenType, tokenType2tokenStr[i]);
		ref->value = calloc(1, sizeof(TokenType));
		*(int*)(ref->value) = i;
	}
}

void loadTransitions(FILE* fp)
{
	transitions = calloc(num_states, sizeof(int**));
	
	for (int i = 0; i < num_states; ++i)
	{
		transitions[i] = calloc(128, sizeof(int*));

		for (int j = 0; j < 128; ++j)
			transitions[i][j] = -1;
	}
	
	for (int i = 0; i < num_transitions; ++i)
	{
		int from, to;
		char BUFF[64];
	
		fscanf(fp, "%d %d %s\n", &from, &to, BUFF);

		for (int j = 0; j < 64; ++j)
		{
			if (BUFF[j] == '\0')
				break;

			transitions[from][BUFF[j]] = to;
		}
	}

	// Comment
	for (int i = 0; i < 128; i++)
		transitions[48][i] = 48;

	transitions[48]['\n'] = 49;

	// White spaces
	transitions[0][' '] =
		transitions[0]['\t'] =
		transitions[0]['\r'] =
		transitions[0]['\n'] = 50;

	transitions[50][' '] =
		transitions[50]['\t'] =
		transitions[50]['\r'] =
		transitions[50]['\n'] = 50;
}

void loadFinalStates(FILE* fp)
{
	finalStates = calloc(num_states, sizeof(TokenType));
	for (int i = 0; i < num_states; ++i)
		finalStates[i] = -1;

	for (int i = 0; i < num_finalstates; i++)
	{
		int state;
		char BUFF[64];
		fscanf(fp, "%d %s\n", &state, BUFF);

		finalStates[state] = *(int*)trie_getVal(tokenStr2tokenType, BUFF);
	}
}

void loadKeywords(FILE* fp)
{
	if (symbolTable == NULL)
		symbolTable = calloc(num_tokens, sizeof(char*));

	for (int i = 0; i < num_keywords; ++i)
	{
		char BUFF1[64], BUFF2[64];
		fscanf(fp, "%s %s\n", BUFF1, BUFF2);

		TrieNode* ref = trie_getRef(symbolTable, BUFF1);
		TokenType val = trie_getVal(tokenStr2tokenType, BUFF2);
		assert(val != NULL);

		ref->value = calloc(1, sizeof(TokenType));
		*(int*)(ref->value) = val;
	}
}

void loadLexer()
{
	if (loaded)
		return;

	FILE* fp = fopen(DEFINITION_LOC, "r");
	assert(fp != NULL);

	fscanf(fp, "%d %d %d %d %d\n", &num_tokens, &num_states, &num_transitions, &num_finalstates, &num_keywords);
	loadTokens(fp);
	loadTransitions(fp);
	loadFinalStates(fp);
	loadKeywords(fp);
}

void loadFile(FILE* fp)
{
	if (b != NULL)
	{
		free(b->archived);
		free(b->working);
		free(b);
	}

	b = calloc(1, sizeof(Buffer));
	b->working = calloc(TWIN_BUFF_SIZE, sizeof(char));
	b->archived = calloc(TWIN_BUFF_SIZE, sizeof(char));
	b->fp = fp;
}

Token* getNextToken()
{
	
}

void removeComments(FILE* source, FILE* destination)
{
	char c;
	int is_comment = 0;
	while (fscanf(source, "%c", &c) != EOF)
	{
		if (c == '%')
			is_comment = 1;

		if (is_comment && (c == '\n' || c == '\r'))
			is_comment = 0;

		if (!is_comment)
			fprintf(destination, "%c", c);
	}
}

void deallocateLexer()
{
	if (b != NULL)
	{
		free(b->archived);
		free(b->working);
		free(b);
		b = NULL;
	}
	loaded = 0;
}