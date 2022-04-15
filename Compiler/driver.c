/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/
// TODO: change all temp and bruh
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#include "DriverHelper.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "symbolTable.h"
#include "typeChecker.h"
#include "IRGenerator.h"
#include "logger.h"

#define MAX_OPTIONS 11

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

void main(int argc, char **argv)
{
	clear_screen();
	loadLexer();
	loadParser();

	if (argc != 3)
	{
		fprintf(stderr, "usage: stage1exe <source_code_file> <output_asm_file>\n");
		exit(-1);
	}

	freopen("compiler.log", "w", stderr);

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

		printf("Compiler menu for group 8\n");
		printf("0: Exit\n");
		printf("1: Print tokens\n");
		printf("2: Print Parse Tree\n");
		printf("3: Print Abstract Syntax Tree\n");
		printf("4: Display Compression\n");
		printf("5: Print Symbol Table\n");
		printf("6: Global Variables\n");
		printf("7. Activation Record Sizes\n");
		printf("8. Record Types and Sizes\n");
		printf("11: Compile\n\n");

		printf("Select an option: ");
		scanf("%d", &option);
		while ((c = getchar()) != '\n' && c != EOF)
			;
		clear_screen();

		if (option == 0)
		{
			freeLexerData();
			printf("Bye bye from group 8 compiler\n");
			break;
		}
		else if (option == 1)
			printLexerOutput(stdout, argv[1]);
		else if (option == 2)
		{
			TreeNode* node = parseInputSourceCode(argv[1]);
			printParseTree(stdout, node);
			freeParseTree(node);
		}
		else if (option == 3)
		{
			TreeNode *node = parseInputSourceCode(argv[1]);
			ASTNode *ast = createAST(node);
			
			printAST(stdout, ast, 0);

			freeAST(ast);
			freeParseTree(node);
		}
		else if (option == 4)
		{
			TreeNode *node = parseInputSourceCode(argv[1]);
			ASTNode *ast = createAST(node);

			printCompression(stdout, node, ast);

			freeAST(ast);
			freeParseTree(node);
			continue;
		}
		else if (option == 5)
		{
			TreeNode *node = parseInputSourceCode(argv[1]);
			ASTNode *ast = createAST(node);

			loadSymbolTable(ast);
			typeChecker_init();
			assignTypes(ast);
			calculateOffsets(ast);

			iterateTrie(globalSymbolTable, printFunctions);

			freeAST(ast);
			freeParseTree(node);
		}
		else if (option == 6)
		{
			TreeNode *node = parseInputSourceCode(argv[1]);
			ASTNode *ast = createAST(node);

			loadSymbolTable(ast);
			typeChecker_init();
			assignTypes(ast);
			calculateOffsets(ast);

			iterateTrie(globalSymbolTable, printVariables);
			
			freeAST(ast);
			freeParseTree(node);
		}
		else if (option == 7)
		{
			TreeNode* node = parseInputSourceCode(argv[1]);
			ASTNode* ast = createAST(node);

			loadSymbolTable(ast);
			typeChecker_init();
			assignTypes(ast);
			calculateOffsets(ast);

			iterateTrie(globalSymbolTable, printActivationRecSize);

			freeAST(ast);
			freeParseTree(node);
		}
		else if (option == 8)
		{
			TreeNode* node = parseInputSourceCode(argv[1]);
			ASTNode* ast = createAST(node);

			loadSymbolTable(ast);
			typeChecker_init();
			assignTypes(ast);
			calculateOffsets(ast);

			iterateTrie(globalSymbolTable, printRecordDetails);

			freeAST(ast);
			freeParseTree(node);
		}
		else if (option == 11)
		{
			TreeNode *node = parseInputSourceCode(argv[1]);
			ASTNode *ast = createAST(node);

			loadSymbolTable(ast);
			typeChecker_init();
			assignTypes(ast);
			calculateOffsets(ast);
			
			ASTNode *func = ast->children[0] == NULL ? ast->children[1] : ast->children[0];
		}
		else
			printf("Invalid option selected: %d\n", option);

	} while (option != 0);
}

/*if (option == 4)
{
	start_time = clock();
	TreeNode* node = parseInputSourceCode(argv[1]);
	end_time = clock();
	double total_CPU_time = (double)(end_time - start_time);
	printf("Total CPU Time Taken: %f\n", total_CPU_time);
	printf("Total CPU Time taken in seconds: %f\n", total_CPU_time / CLOCKS_PER_SEC);
	freeParseTree(node);
	continue;
}
*/