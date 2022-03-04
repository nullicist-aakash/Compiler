/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/


#ifndef LEXERDEF_H
#define LEXERDEF_H
#include "trie.h"
#include "stdio.h"

typedef enum TokenType
{
	TK_ASSIGNOP,
	TK_COMMENT,
	TK_FIELDID,
	TK_ID,
	TK_NUM,
	TK_RNUM,
	TK_FUNID,
	TK_RUID,
	TK_WITH,
	TK_PARAMETERS,
	TK_END,
	TK_WHILE,
	TK_UNION,
	TK_ENDUNION,
	TK_DEFINETYPE,
	TK_AS,
	TK_TYPE,
	TK_MAIN,
	TK_GLOBAL,
	TK_PARAMETER,
	TK_LIST,
	TK_SQL,
	TK_SQR,
	TK_INPUT,
	TK_OUTPUT,
	TK_INT,
	TK_REAL,
	TK_COMMA,
	TK_SEM,
	TK_COLON,
	TK_DOT,
	TK_ENDWHILE,
	TK_OP,
	TK_CL,
	TK_IF,
	TK_THEN,
	TK_ENDIF,
	TK_READ,
	TK_WRITE,
	TK_RETURN,
	TK_PLUS,
	TK_MINUS,
	TK_MUL,
	TK_DIV,
	TK_CALL,
	TK_RECORD,
	TK_ENDRECORD,
	TK_ELSE,
	TK_AND,
	TK_OR,
	TK_NOT,
	TK_LT,
	TK_LE,
	TK_EQ,
	TK_GT,
	TK_GE,
	TK_NE,
	TK_WHITESPACE,
	TK_ERROR_SYMBOL,
	TK_ERROR_PATTERN,
	TK_ERROR_LENGTH
} TokenType;

typedef struct Token
{
	enum TokenType type;
	char* lexeme;
	int line_number;
	int start_index;
	int length;
} Token;

typedef struct
{
	int num_tokens;
	int num_states;
	int num_transitions;
	int num_finalstates;
	int num_keywords;

	int** productions;
	TokenType* finalStates;
	char** tokenType2tokenStr;
	char** keyword2Str;
	Trie* tokenStr2tokenType;
	Trie* symbolTable;	// move to global in future

} LexerData;

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

#define TWIN_BUFF_SIZE 50
#define DEFINITION_LOC "DFA_Structure.txt"

#endif
