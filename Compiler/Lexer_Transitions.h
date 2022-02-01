#ifndef LEXER_TRANSITIONS_H
#define LEXER_TRANSITIONS_H
#include "Lexer.h"

#define NUM_TRANSITIONS 1
#define KEYWORD_COUNT	33
#define SYMBOL_COUNT	21
#define OPERATOR_COUNT	36

char* keywords[KEYWORD_COUNT], * symbols[SYMBOL_COUNT], * operators[OPERATOR_COUNT];

TokenType isKeyword(char*, int*);

TokenType isNumber(char*, int*);

TokenType(*transitions[])(char*, int*);
#endif