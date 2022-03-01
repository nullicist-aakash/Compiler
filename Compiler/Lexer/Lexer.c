#include "Lexer.h"
#include "../helpers/Trie.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TWIN_BUFF_SIZE 50
#define DEFINITION_LOC "./DFA_Structure.txt"

typedef struct
{
	FILE* fp;
	char* working;
	char* archived;

	int charTaken;
	int line_number;
	int start_index;

} Buffer;

LexerData* lexerData;
Buffer* b;

void loadTokens(FILE* fp)
{
	lexerData->tokenType2tokenStr = calloc(lexerData->num_tokens, sizeof(char*));
	lexerData->tokenStr2tokenType = calloc(1, sizeof(Trie));

	for (int i = 0; i < lexerData->num_tokens; ++i)
	{
		char BUFF[64];
		fscanf(fp, "%s\n", BUFF);
		lexerData->tokenType2tokenStr[i] = calloc(strlen(BUFF) + 1, sizeof(char));
		strcpy(lexerData->tokenType2tokenStr[i], BUFF);

		TrieNode* ref = trie_getRef(lexerData->tokenStr2tokenType, lexerData->tokenType2tokenStr[i]);
		ref->entry.value = i;
	}
}

void loadTransitions(FILE* fp)
{
	lexerData->transitions = calloc(lexerData->num_states, sizeof(int**));

	for (int i = 0; i < lexerData->num_states; ++i)
	{
		lexerData->transitions[i] = calloc(128, sizeof(int*));

		for (int j = 0; j < 128; ++j)
			lexerData->transitions[i][j] = -1;
	}

	for (int i = 0; i < lexerData->num_transitions; ++i)
	{
		int from, to;
		char BUFF[64];

		fscanf(fp, "%d %d %s\n", &from, &to, BUFF);

		for (int j = 0; j < 64; ++j)
		{
			if (BUFF[j] == '\0')
				break;

			lexerData->transitions[from][BUFF[j]] = to;
		}
	}

	// Comment
	for (int i = 0; i < 128; i++)
		lexerData->transitions[48][i] = 48;

	lexerData->transitions[48]['\n'] = -1;

	// White spaces
	lexerData->transitions[0][' '] =
		lexerData->transitions[0]['\t'] =
		lexerData->transitions[0]['\r'] =
		lexerData->transitions[0]['\n'] = 50;

	lexerData->transitions[50][' '] =
		lexerData->transitions[50]['\t'] =
		lexerData->transitions[50]['\r'] =
		lexerData->transitions[50]['\n'] = 50;
}

void loadFinalStates(FILE* fp)
{
	lexerData->finalStates = calloc(lexerData->num_states, sizeof(TokenType));
	for (int i = 0; i < lexerData->num_states; ++i)
		lexerData->finalStates[i] = -1;

	for (int i = 0; i < lexerData->num_finalstates; i++)
	{
		int state;
		char BUFF[64];
		fscanf(fp, "%d %s\n", &state, BUFF);

		lexerData->finalStates[state] = trie_getVal(lexerData->tokenStr2tokenType, BUFF).value;
	}
}

void loadKeywords(FILE* fp)
{
	if (lexerData->symbolTable == NULL)
		lexerData->symbolTable = calloc(lexerData->num_tokens, sizeof(char*));

	for (int i = 0; i < lexerData->num_keywords; ++i)
	{
		char BUFF1[64], BUFF2[64];
		fscanf(fp, "%s %s\n", BUFF1, BUFF2);

		TrieNode* ref = trie_getRef(lexerData->symbolTable, BUFF1);
		ref->entry.value = trie_getVal(lexerData->tokenStr2tokenType, BUFF2).value;
	}
}

char getChar(int i)
{
	if (b->charTaken == i)
	{
		char* temp = b->archived;
		b->archived = b->working;
		b->working = temp;

		char c = 0;
		int reads = 0;
		while (reads != TWIN_BUFF_SIZE && fscanf(b->fp, "%c", &c) != EOF)
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

void loadLexer()
{
	FILE* fp = fopen("./Lexer/DFA_Structure.txt", "r");
	lexerData = calloc(1, sizeof(LexerData));

	assert(fp != NULL);

	fscanf(fp, "%d %d %d %d %d\n", &lexerData->num_tokens, &lexerData->num_states, &lexerData->num_transitions, &lexerData->num_finalstates, &lexerData->num_keywords);
	loadTokens(fp);
	loadTransitions(fp);
	loadFinalStates(fp);
	loadKeywords(fp);

	fclose(fp);
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
	b->line_number = 1;
}

Token* DFA(int start_index)
{
	TokenType ttype;
	int last_final = -1;
	int input_final_pos = start_index - 1;

	int len = 0;

	int cur_state = 0;
	// start index = index of character to read next

	while (1)
	{
		char input = getChar(start_index);

		if (lexerData->finalStates[cur_state] != -1)
		{
			last_final = cur_state;
			ttype = lexerData->finalStates[cur_state];
			input_final_pos = start_index - 1;
		}

		cur_state = lexerData->transitions[cur_state][input];

		//if (cur_state == 49)
		//	b->line_number++;

		if (cur_state == -1)    // return
		{
			if (input_final_pos == start_index - len - 1)
			{
				Token* token = calloc(1, sizeof(Token));
				token->type = TK_ERROR_SYMBOL;
				token->length = 1;
				return token;
			}

			Token* token = calloc(1, sizeof(Token));
			token->type = ttype;
			token->length = input_final_pos - (start_index - len) + 1;
			return token;
		}

		start_index++;
		len++;
	}

	// this should not be reachable as our DFA is capable of handling every case
	assert(0);
}

Token* getNextToken()
{
	while (getChar(b->start_index) != '\0')
	{
		if (getChar(b->start_index) == '\n')
		{
			b->line_number++;
			b->start_index++;
			continue;
		}

		Token* token = DFA(b->start_index);

		assert(token != NULL);

		token->start_index = b->start_index;
		token->line_number = b->line_number;

		b->start_index += token->length;

		if (token->type == TK_COMMENT || token->type == TK_WHITESPACE)
		{
			free(token);
			continue;
		}

		token->lexeme = calloc(token->length + 1, sizeof(char));

		for (int i = 0; i < token->length; i++)
			token->lexeme[i] = getChar(b->start_index - token->length + i);

		if (token->type == TK_ID || token->type == TK_FUNID || token->type == TK_FIELDID)
		{
			TrieNode* temp = trie_getRef(lexerData->symbolTable, token->lexeme);

			if (temp->entry.value)
				token->type = temp->entry.value;
		}

		// Assign error types

		if (token->type == TK_ID && token->length > 20)
			token->type = TK_ERROR_LENGTH;

		if (token->type == TK_FUNID && token->length > 30)
			token->type = TK_ERROR_LENGTH;

		return token;
	}

	return NULL;
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