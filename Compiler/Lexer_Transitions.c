#include "Lexer_Transitions.h"

TokenType(*transitions[])(char*, int*) = { isNumber, isKeyword };

TokenType isKeyword(char* stream, int* size)
{
	*size = 4;
	return KEYWORD;
}

TokenType isNumber(char* stream, int* size)
{
	*size = 5;
	return IDENTIFIER;
}