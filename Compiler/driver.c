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

	if (argc != 2)
	{
		fprintf(stderr, "usage: stage1exe <source_code_file>\n");
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

		printf("Following are the options to select from\n");
		printf("0: Exit\n");
		printf("1: Print token list generated by lexer\n");
		printf("2: Parse the source code and print Parse Tree to console\n");
		printf("3: Print Abstract Syntax Tree to console using DFS\n");
		printf("4: Display Compression\n");
		printf("5: Print Symbol Table\n");
		printf("6: Global Variables\n");
		printf("7. Activation Record Sizes\n");
		printf("8. Record Types and Sizes\n");
		printf("11: Compile\n");

		printf("Select an option: ");
		scanf("%d", &option);
		while ((c = getchar()) != '\n' && c != EOF)
			;
		clear_screen();

		if (option < 0 || option > MAX_OPTIONS)
		{
		}

		if (option == 0)
		{
			freeLexerData();
			printf("Bye bye from group 8 compiler\n");
			break;
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
		else if (option == 1)
			printLexerOutput(argv[1]);
		else if (option == 2)
		{
			TreeNode* node = parseInputSourceCode(argv[1]);

			// FILE* fptr = fopen(argv[2], "w");
			printf("Group 8 Output File.\n%25s %14s %11s %18s %24s %12s %29s\n",
				   "Lexeme",
				   "LineNumber",
				   "TokenName",
				   "Value (if Num)",
				   "ParentSymbol",
				   "Is Leaf",
				   "NodeSymbol");

			printParseTree(node);

			freeParseTree(node);
		}
		else if (option == 3)
		{
			TreeNode *node = parseInputSourceCode(argv[1]);
			ASTNode *ast = createAST(node);
			printAST(ast, 0);
		}
		else if (option == 4)
		{
			TreeNode *node = parseInputSourceCode(argv[1]);
			ASTNode *ast = createAST(node);

			int numParseTreeNodes = 0;
			int parseTreeSize = 0;
			int numASTNodes = 0;
			int ASTSize = 0;

			ParseTreeDfs(node, &numParseTreeNodes, &parseTreeSize);
			ASTDfs(ast, &numASTNodes, &ASTSize);

			printf("Parse tree Number of nodes = %d Allocated Memory = %d Bytes\n", numParseTreeNodes, parseTreeSize);
			printf("AST Number of nodes = %d Allocated Memory = %d Bytes\n", numASTNodes, ASTSize);
			float Compression = (float)(parseTreeSize - ASTSize) / parseTreeSize * 100;
			printf("Compression percentage = ((%d - %d) / %d) * 100 = %.2f%c\n", parseTreeSize, ASTSize, parseTreeSize, Compression, '%');
		}
		else if (option == 5)
		{
			TreeNode *node = parseInputSourceCode(argv[1]);
			ASTNode *ast = createAST(node);
			loadSymbolTable(ast);
			calculateOffsets(ast);

			iterateTrie(globalSymbolTable, printGlobalSymbolTable);	   // Iterating global symbol table to print all global variables
			iterateTrie(globalSymbolTable, printFunctionSymbolTables); // Iterating global symbol table to print local symbol tables
		}
		else if (option == 6)
		{
			TreeNode *node = parseInputSourceCode(argv[1]);
			ASTNode *ast = createAST(node);
			loadSymbolTable(ast);
			calculateOffsets(ast);

			iterateTrie(globalSymbolTable, printGlobalSymbolTable);
		}
		else if (option == 7)
		{
			TreeNode* node = parseInputSourceCode(argv[1]);
			ASTNode* ast = createAST(node);
			loadSymbolTable(ast);
			calculateOffsets(ast);

			iterateTrie(globalSymbolTable, printFunctionActivationRecordSize);
		}
		else if (option == 8)
		{
			TreeNode* node = parseInputSourceCode(argv[1]);
			ASTNode* ast = createAST(node);
			loadSymbolTable(ast);
			iterateTrie(globalSymbolTable, printRecordDetails);
		}
		else if (option == 11)
		{
			TreeNode *node = parseInputSourceCode(argv[1]);
			// printParseTree(node);

			ASTNode *ast = createAST(node);

			loadSymbolTable(ast);

			typeChecker_init();
			assignTypes(ast);
			logIt("Type Checking Completed ==========\n");

			logIt("Generating code for functions ==========\n");

			ASTNode *func = ast->children[0] == NULL ? ast->children[1] : ast->children[0];

			// while (func)
			// {
			// 	IRInsNode *code = generateFuncCode(func)->head;

			// 	while (code)
			// 	{
			// 		if (code->ins->op == OP_JMP)
			// 			logIt("\tJMP Label#%d\n", code->ins->dst.int_val);
			// 		else if (code->ins->op == OP_LABEL)
			// 			logIt("Label#%d:\n", code->ins->dst.int_val);
			// 		else if (code->ins->op == OP_ASSIGN)
			// 			logIt("\t%s = %s\n", code->ins->dst.name, code->ins->src1.name);
			// 		else if (code->ins->op == OP_STORE_INT)
			// 			logIt("\t%s = %d\n", code->ins->dst.name, code->ins->src1.int_val);
			// 		else if (code->ins->op == OP_STORE_REAL)
			// 			logIt("\t%s = %f\n", code->ins->dst.name, code->ins->src1.real_val);
			// 		else if (code->ins->op == OP_ADD)
			// 			logIt("\t%s = %s + %s\n", code->ins->dst.name, code->ins->src1.name, code->ins->src2.name);
			// 		else if (code->ins->op == OP_SUB)
			// 			logIt("\t%s = %s - %s\n", code->ins->dst.name, code->ins->src1.name, code->ins->src2.name);
			// 		else if (code->ins->op == OP_MUL)
			// 			logIt("\t%s = %s * %s\n", code->ins->dst.name, code->ins->src1.name, code->ins->src2.name);
			// 		else if (code->ins->op == OP_DIV)
			// 			logIt("\t%s = %s / %s\n", code->ins->dst.name, code->ins->src1.name, code->ins->src2.name);
			// 		else if (code->ins->op == OP_LE)
			// 			logIt("\tif %s <= %s, JMP Label#%d\n", code->ins->src1.name, code->ins->src2.name, code->ins->dst.int_val);
			// 		else if (code->ins->op == OP_LT)
			// 			logIt("\tif %s < %s, JMP Label#%d\n", code->ins->src1.name, code->ins->src2.name, code->ins->dst.int_val);
			// 		else if (code->ins->op == OP_GE)
			// 			logIt("\tif %s >= %s, JMP Label#%d\n", code->ins->src1.name, code->ins->src2.name, code->ins->dst.int_val);
			// 		else if (code->ins->op == OP_GT)
			// 			logIt("\tif %s > %s, JMP Label#%d\n", code->ins->src1.name, code->ins->src2.name, code->ins->dst.int_val);
			// 		else if (code->ins->op == OP_EQ)
			// 			logIt("\tif %s == %s, JMP Label#%d\n", code->ins->src1.name, code->ins->src2.name, code->ins->dst.int_val);
			// 		else if (code->ins->op == OP_NEQ)
			// 			logIt("\tif %s != %s, JMP Label#%d\n", code->ins->src1.name, code->ins->src2.name, code->ins->dst.int_val);
			// 		else if (code->ins->op == OP_READ)
			// 			logIt("\tRead %s\n", code->ins->dst.name);
			// 		else if (code->ins->op == OP_WRITE)
			// 			logIt("\tWrite %s\n", code->ins->dst.name);
			// 		else
			// 			assert(0);

			// 		code = code->next;
			// 	}

			// 	if (func->sibling == NULL && func != ast->children[1])
			// 		func = ast->children[1];
			// 	else
			// 		func = func->sibling;
			// }
		}
		else
		{
			printf("Invalid option selected: %d\n", option);
			continue;
		}

	} while (option != 0);
}
