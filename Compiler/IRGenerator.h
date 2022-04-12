#pragma once
#include "typeChecker.h"

typedef struct IRInsList IRInsList;

typedef enum
{
	OP_LOAD,
	OP_STORE,
	OP_JMP,
	OP_LABEL,
	OP_ASSIGN,
	OP_STORE_INT,		// store integer to destination
	OP_STORE_REAL,		// store real number to destination

	// arithmetic
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	
	// logical
	OP_AND,
	OP_OR,
	OP_NOT,

	// relational
	OP_LE,
	OP_GE,
	OP_LT,
	OP_GT,
	OP_EQ,
	OP_NEQ,

	// IO
	OP_READ,
	OP_WRITE
} IROPType;

typedef enum
{
	PAYLOAD_LABEL,
	PAYLOAD_BOOL,
	PAYLOAD_ARITH,
	PAYLOAD_STMT
} IRPayloadType;

typedef struct
{
	int label_no;
} LabelPayload;

typedef struct
{
	IRInsList* code;
	LabelPayload _true;
	LabelPayload _false;
} BoolPayload;

typedef struct
{
	IRInsList* code;
	LabelPayload label_next;
} StmtPayload;

typedef struct
{
	char* name;
	IRInsList* code;
} ExpPayload;

typedef union
{
	LabelPayload _label;
	BoolPayload _bool;
	StmtPayload _stmt;
	ExpPayload _arith;
} PayloadUnion;

typedef struct
{
	IRPayloadType payload_type;
	PayloadUnion payload;
} Payload;

typedef union
{
	char* name;
	int int_val;
	float real_val;
} Operand;

typedef struct
{
	IROPType op;
	Operand src1;
	Operand src2;
	Operand dst;
} IRInstr;

typedef struct IRInsNode
{
	IRInstr* ins;
	struct IRInsNode* next;
} IRInsNode;

struct IRInsList
{
	IRInsNode* head;
	IRInsNode* tail;
};

IRInsList* generateFuncCode(ASTNode*);