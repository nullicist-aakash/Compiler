/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/

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
int empty(Stack*);

#endif