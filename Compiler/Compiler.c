#include <stdio.h>
#include "./Lexer/Lexer.h"
#include "helpers/Globals.h"

int main()
{
	for (int i = 0; i < 51; ++i)
	{
		printf("case %d: break;\n", i);
	}
	// loadCode("./data/Code.txt");
}