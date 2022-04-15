/***************************************
        GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/
#pragma once
#include "lexerDef.h"

extern LexerData *lexerData;

void loadLexer();

void loadFile(FILE *);

Token *getNextToken();

void removeComments(FILE *source, FILE *destination);
void printLexerOutput(char *);
void freeLexerData();
