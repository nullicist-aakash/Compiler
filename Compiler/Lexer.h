#pragma once
#include "Trie.h"
#include "lexerDef.h"


extern LexerData* lexerData;

void loadLexer();

void loadFile(FILE*);

Token* getNextToken();

void removeComments(FILE* source, FILE* destination);

void freeLexerData();
