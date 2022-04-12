#pragma once
#include "typeChecker.h"

struct IRInsNode;

typedef enum
{
	OP_LOAD,
	OP_STORE,
	OP_JMP,
	OP_IF,
	OP_LABEL,

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
	IRInsNode* code;
	LabelPayload _true;
	LabelPayload _false;
} BoolPayload;

typedef struct
{
	IRInsNode* code;
	LabelPayload label_next;
} StmtPayload;

typedef struct
{
	char* name;
	IRInsNode* code;
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
} OperandType;

typedef struct IRInstruction
{
	IROPType op;
	OperandType src1;
	OperandType src2;
	OperandType dst;
} IRInstruction;

typedef struct IRInsNode
{
	IRInstruction* ins;
	struct IRInsNode* next;
} IRInsNode;

typedef struct
{
	IRInsNode* head;
	IRInsNode* tail;
} IRInsList;

IRInsList* generateFuncCode(ASTNode*);