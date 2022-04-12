#pragma once
#include "typeChecker.h"
extern char* IRCodeLoc;

typedef enum
{
	ADD,
	SUB,
	MUL,
	DIV,
	ADDF,
	SUBF,
	MULF,
	DIVF,
	GE,
	LE,
	GT,
	LT,
	EQ,
	NEQ,
	AND,
	OR,
	NOT,
	LOAD,
	STORE,
	JMP,
	JMPC,
	READ,
	WRITE,
	LABEL,
	CALL,
	RET
}OpType;

typedef struct IRCode
{
	OpType op;
	char* dest;
	char* s1;
	char* s2;
	int offset;
}IRCode;

void generateFunctionCode(ASTNode*);