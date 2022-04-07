/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/

#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void push(Stack* stack, int data)
{
	StackNode* ptr = calloc(1, sizeof(StackNode));
	ptr->prev = stack->top;
	ptr->data = data;
	stack->top = ptr;
}

int top(Stack* stack)
{
	assert(stack->top != NULL);
	return stack->top->data;
}

void pop(Stack* stack)
{
	assert(stack->top != NULL);
	void* new_top = stack->top->prev;
	free(stack->top);

	stack->top = new_top;
}

int empty(Stack* stack)
{
	return stack->top == NULL;
}