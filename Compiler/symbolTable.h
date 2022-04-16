/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/

#include "symbolTableDef.h"
#pragma once

extern int symbolTableErr;

void loadSymbolTable(ASTNode *);
void printGlobalSymbolTable(char*, TrieEntry *);
void printFunctionSymbolTables(char*, TrieEntry*);
void printFunctionActivationRecordSize(char*, TrieEntry*);
void printRecordDetails(char*, TrieEntry*);
void printLocalTable(char *, TrieEntry *); //TODO:remove this
