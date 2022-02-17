#ifndef LEXER_H
#define LEXER_H

#define MAX_CODE_SIZE 65536

enum TokenType
{
	WHITESPACE = 0,
	KEYWORD = 1,
	OPERATOR = 2,
	SYMBOL = 3,
	IDENTIFIER = 4,
	COMMENT = 5
};

// Todo: 
struct Token
{
	enum TokenType type;
	char* lexeme;
	int line_number;
	int start_index;
	int length;
};

struct TokenNode
{
	struct Token* token;
	struct TokenNode* next;
};

typedef enum TokenType TokenType;
typedef struct Token Token;
typedef struct TokenNode TokenNode;

void loadCode();
TokenNode* getTokens();

#endif