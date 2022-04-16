#include "symbolTableDef.h"
#pragma once

void loadSymbolTable(ASTNode *);
void printGlobalSymbolTable(char*, TrieEntry *);
void printFunctionSymbolTables(char*, TrieEntry*);
void printFunctionActivationRecordSize(char*, TrieEntry*);
void printRecordDetails(char*, TrieEntry*);
void printLocalTable(char *, TrieEntry *); //TODO:remove this
