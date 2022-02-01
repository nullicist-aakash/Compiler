#ifndef LEXER_H
#define LEXER_H


#define KEYWORD_COUNT	33
#define SYMBOL_COUNT	21
#define OPERATOR_COUNT	36
#define MAX_CODE_SIZE 65536

typedef enum
{
	NO_ERROR = 0,
	FILE_READ_ERROR = 1
} ErrorCode;

typedef enum
{
	KEYWORD = 0,
	OPERATOR = 1,
	SYMBOL = 2,
	IDENTIFIER = 3,
	COMMENT = 4,
	UNKNOWN = 5
} TokenType;

typedef struct
{
	TokenType t;
	int line_number;
	int start_index;
	int length;

} Token;

void loadDataFiles();
void loadCode(char* loc);

#endif