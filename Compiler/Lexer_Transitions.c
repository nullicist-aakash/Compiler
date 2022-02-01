#include "Lexer_Transitions.h"

int isKeyword(char* stream, int* size)
{
	*size = 4;
	return 1;
}

int isNumber(char* stream, int* size)
{
	*size = 5;
	return 1;
}