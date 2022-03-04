/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  NIHIR AGARWAL			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h> 

#include "Lexer.h"
#include "Parser.h"

#define MAX_OPTIONS 4

void clear_screen()
{
#ifdef _WIN32
	system("cls");
#endif
#ifdef _WIN64
	system("cls");
#endif
#ifdef __unix__
	system("clear");
#endif
#ifdef __linux__
	system("clear");
#endif
}

void printLexerOutput(char* path)
{
	FILE* fp = fopen(path, "r");

	if (!fp)
	{
		fprintf(stderr, "Error opening file %s: %s\n", path, strerror(errno));
		return;
	}

	loadFile(fp);

	Token* tk;
	while ((tk = getNextToken()) != NULL)
	{
		if (tk->type == TK_ERROR_LENGTH)
			printf("Line no. %d: Error: Identifier length is greater than the prescribed length.\n", tk->line_number);
		else if (tk->type == TK_ERROR_SYMBOL)
			printf("Line no. %d: Error: Unknwon Symbol <%s>\n", tk->line_number, tk->lexeme);
		else if (tk->type == TK_ERROR_PATTERN)
			printf("Line no. %d: Error: Unknown Pattern <%s>\n", tk->line_number, tk->lexeme);
		else
			printf("Line no. %d\tLexeme %s\t\tToken %s\n", tk->line_number, tk->lexeme, lexerData->tokenType2tokenStr[tk->type]);
	}
}

void traverse(TreeNode* node)
{
	if (node->parent != NULL)
		printf("Node: %s, parent: %s\n", parserData->symbolType2symbolStr[node->symbol_index], parserData->symbolType2symbolStr[node->parent->symbol_index]);
	else
		printf("On root\n");

	if (node->parent != NULL)
	{
		char* A = node->isLeaf ? node->token->lexeme : "----";
		int B = node->isLeaf ? node->token->line_number : -1;
		char* C = !(node->isLeaf) ? "----" : parserData->symbolType2symbolStr[node->symbol_index];
		int D = 0;
		char* E = node->parent == NULL ? "root" : parserData->symbolType2symbolStr[node->parent->symbol_index];
		char* F = node->isLeaf ? "yes" : "no";
		char* G = node->isLeaf ? "----" : parserData->symbolType2symbolStr[node->symbol_index];
		printf("%30s %10d %30s %5d %30s %10s %30s\n", A, B, C, D, E, F, G);
	}
	else
		printf("Root\n");

	for (int i = 0; i < node->child_count; ++i)
		traverse(node->children[i]);
}

int main(int argc, char** argv)
{
	clear_screen();
	loadLexer();
	loadParser();
	/*
	if (argc != 3)
	{
		fprintf(stderr, "Argument list is not valid!\n");
		exit(-1);
	}*/

	argv[1] = "t1.txt";

	int option = 0;
	int start = 1;
	char c;

	clock_t start_time = clock();
	clock_t end_time = start_time;

	do
	{
		if (!start)
		{
			printf("Press any key to continue...");
			scanf("%c", &c);
			clear_screen();
		}
		start = 0;

		printf("Following are the options to select from\n");
		printf("0: Exit\n");
		printf("1: Remove Comments\n");
		printf("2: Print lexer output\n");
		printf("3: Parse the code\n");
		printf("4: Print the time taken in last option\n");
		printf("Select an option: ");
		scanf("%d", &option);
		while ((c = getchar()) != '\n' && c != EOF);
		clear_screen();

		if (option < 0 || option > MAX_OPTIONS)
		{
			printf("Invalid option selected: %d\n", option);
			continue;
		}

		if (option == 0)
		{
			printf("Bye bye from group 8 compiler\n");
			break;
		}
		if (option == MAX_OPTIONS)
		{
			double total_CPU_time = (double)(end_time - start_time);
			printf("Total CPU Time Taken: %f\n", total_CPU_time);
			printf("Total CPU Time taken in seconds: %f\n", total_CPU_time / CLOCKS_PER_SEC);
			continue;
		}

		start_time = clock();

		if (option == 1)
		{
			FILE* source = fopen(argv[1], "r");

			if (source == NULL)
			{
				fprintf(stderr, "Error opening file %s: %s\n", argv[1], strerror(errno));
				return;
			}

			removeComments(source, stdout);

			fclose(source);
		}
		else if (option == 2)
			printLexerOutput(argv[1]);
		else if (option == 3)
		{
			TreeNode* node = parseSourceCode(argv[1]);

			traverse(node);
		}
		end_time = clock();

	} while (option != 0);
}