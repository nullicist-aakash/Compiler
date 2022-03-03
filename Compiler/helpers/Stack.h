#ifndef STACK_H
#define STACK_H

typedef struct StackNode
{
	void* data;
	struct StackNode* prev;
} StackNode;

typedef struct Stack
{
	StackNode* top;
} Stack;

void push(Stack*, void*);
void* top(Stack*);
void pop(Stack*);

#endif