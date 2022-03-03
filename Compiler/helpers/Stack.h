#ifndef STACK_H
#define STACK_H

typedef struct StackNode
{
	int data;
	struct StackNode* prev;
} StackNode;

typedef struct Stack
{
	StackNode* top;
} Stack;

void push(Stack*, int);
int top(Stack*);
void pop(Stack*);

#endif