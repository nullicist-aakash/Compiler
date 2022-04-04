#pragma once

enum TypeTag
{
	NUM,
	RNUM,
	FUNCTION,
	DERIVED,
	BOOL,
	VOID
};

struct TypeInfo;

typedef struct
{
	int argCount;
	TypeInfo** argTypes;

	TypeInfo* retType;
} FuncEntry;

typedef struct
{
	int isUnion;
	TypeInfo* type;
	TypeInfo* next;
} DerivedEntry;

typedef union
{
	FuncEntry func;
	DerivedEntry derived;
} TypeStructure;

typedef struct 
{
	TypeTag entryType;
	TypeStructure val;
} TypeInfo;