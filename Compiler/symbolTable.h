#include "symbolTableDef.h"
#pragma once

void loadSymbolTable(ASTNode *);
void printGlobalSymbolTable(TrieEntry *);
void printFunctionSymbolTables(TrieEntry*);
void printFunctionActivationRecordSize(TrieEntry*);
void printRecordDetails(TrieEntry*);