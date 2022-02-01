#ifndef LEXER_TRANSITIONS_H
#define LEXER_TRANSITIONS_H
#include "Lexer.h"

int isKeyword(char*, int*);

int isNumber(char*, int*);

int (*transisitons[2])(char*, int*) = { isNumber, isKeyword };
#endif