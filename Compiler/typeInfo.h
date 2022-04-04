#pragma once

enum TypeEntry
{
	NUM,
	RNUM,
	FUNCTION,
	DERIVED,
	ARRAY,
	BOOL,
	VOID
};

struct Type;

typedef struct
{
	int val;
} NumEntry;

typedef struct
{
	float val;
} RNumEntry;

typedef struct
{
	bool val;
} BoolEntry;

typedef struct
{
	int argCount;
	Type** argTypes;

	Type* retType;
} FuncEntry;

typedef struct
{
	int isUnion;
	int fieldCount;
	Type** fields;
} DerivedEntry;

typedef struct
{
	int count;
	Type* type;
} ArrayEntry;

typedef struct Type
{
	TypeEntry entry;
	void* val;
} Type;