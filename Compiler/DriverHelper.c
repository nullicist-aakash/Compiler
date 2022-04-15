#include <errno.h>
#include <string.h>

#include "typeChecker.h"

void removeComments(FILE* source, FILE* destination)
{
	char c;
	int is_comment = 0;
	while (fscanf(source, "%c", &c) != EOF)
	{
		if (c == '%')
			is_comment = 1;

		if (is_comment && (c == '\n' || c == '\r'))
			is_comment = 0;

		if (!is_comment)
			fprintf(destination, "%c", c);
	}
}

void printLexerOutput(FILE* output, char* path)
{
	FILE* fp = fopen(path, "r");

	if (!fp)
	{
		fprintf(output, "Error opening file %s: %s\n", path, strerror(errno));
		return;
	}

	loadFile(fp);

	Token* tk;
	while ((tk = getNextToken()))
	{
		if (tk->type == TK_ERROR_LENGTH)
			fprintf(output, "Line no. %d: Error: Identifier length is greater than the prescribed length.\n", tk->line_number);
		else if (tk->type == TK_ERROR_SYMBOL)
			fprintf(output, "Line no. %d: Error: Unknwon Symbol <%s>\n", tk->line_number, tk->lexeme);
		else if (tk->type == TK_ERROR_PATTERN)
			fprintf(output, "Line no. %d: Error: Unknown Pattern <%s>\n", tk->line_number, tk->lexeme);
		else
			fprintf(output, "Line no. %d\tLexeme %s\t\tToken %s\n", tk->line_number, tk->lexeme, lexerData->tokenType2tokenStr[tk->type]);

		if (tk->lexeme)
			free(tk->lexeme);

		free(tk);
	}

	free(fp);
}

void recursivePrintParseTree(FILE* fp, TreeNode* node)
{
	if (node->parent != NULL)
	{
		char* A = node->isLeaf ? node->token->lexeme : "----";
		int B = node->isLeaf ? node->token->line_number : -1;
		char* C = !(node->isLeaf) ? "----" : parserData->symbolType2symbolStr[node->symbol_index];
		double D = getVal(node->token);
		char* E = node->parent == NULL ? "root" : parserData->symbolType2symbolStr[node->parent->symbol_index];
		char* F = node->isLeaf ? "yes" : "no";
		char* G = node->isLeaf ? "----" : parserData->symbolType2symbolStr[node->symbol_index];

		fprintf(fp, "%25s %10d %15s %15f %27s %10s %27s\n", A, B, C, D, E, F, G);
	}
	else
		fprintf(fp, "%25s %10d %15s %15s %27s %10s %27s\n", "----", -1, "----", "-nan", "ROOT", "no", "program");

	for (int i = 0; i < node->child_count; ++i)
		recursivePrintParseTree(fp, node->children[i]);
}

void printParseTree(FILE* fp, TreeNode* node)
{
	fprintf(fp, "Group 8 Output File.\n%25s %14s %11s %18s %24s %12s %29s\n",
		"Lexeme",
		"LineNumber",
		"TokenName",
		"Value (if Num)",
		"ParentSymbol",
		"Is Leaf",
		"NodeSymbol");

	recursivePrintParseTree(fp, node);
}

void printAST(FILE* fp, ASTNode* node, int tab)
{
	if (node == NULL)
		return;

	for (int i = 0; i < tab; ++i)
		fprintf(fp, "\t");

	char symbol_string[50], lexeme_string[50];
	symbol_string[0] = lexeme_string[0] = '\0';
	char* isGlobal = (node->isGlobal ? ", is Global" : "");

	snprintf(symbol_string, 50, "symbol : %s", parserData->symbolType2symbolStr[node->sym_index]);
	
	if (node->token)
		snprintf(lexeme_string, 50, ", lexeme : %s", node->token->lexeme);
	
	fprintf(fp, "{ %s%s%s }\n", symbol_string, lexeme_string, isGlobal);

	for (int i = 0; i < node->childCount; ++i)
		printAST(fp, node->children[i], tab + 1);

	printAST(fp, node->sibling, tab);
}

void ParseTreeDfs(TreeNode* node, int* num_nodes, int* size)
{
	if (!node)
		return;

	for (int i = 0; i < node->child_count; i++)
		ParseTreeDfs(node->children[i], num_nodes, size);

	(*num_nodes)++;
	(*size) += sizeof(TreeNode) + (node->token ? node->token->length : 0);
}

void ASTDfs(ASTNode* node, int* num_nodes, int* size)
{
	if (!node)
		return;

	for (int i = 0; i < node->childCount; i++)
		ASTDfs(node->children[i], num_nodes, size);

	ASTDfs(node->sibling, num_nodes, size);
	
	(*num_nodes)++;
	(*size) += sizeof(TreeNode) + (node->token ? node->token->length : 0);
}

void printCompression(FILE* fp, TreeNode* node, ASTNode* ast)
{
	int numParseTreeNodes = 0;
	int parseTreeSize = 0;
	int numASTNodes = 0;
	int ASTSize = 0;

	ParseTreeDfs(node, &numParseTreeNodes, &parseTreeSize);
	ASTDfs(ast, &numASTNodes, &ASTSize);

	fprintf(fp, "Parse tree Number of nodes = %d Allocated Memory = %d Bytes\n", numParseTreeNodes, parseTreeSize);
	fprintf(fp, "AST Number of nodes = %d Allocated Memory = %d Bytes\n", numASTNodes, ASTSize);
	float Compression = (float)(parseTreeSize - ASTSize) / parseTreeSize * 100;
	fprintf(fp, "Compression percentage = ((%d - %d) / %d) * 100 = %.2f%c\n", parseTreeSize, ASTSize, parseTreeSize, Compression, '%');
}


FuncEntry* local_func;

void printTypeName(VariableEntry* entry)
{
	TypeTag type = entry->type->entryType;
	if (type == INT || type == REAL)
		printf("---");
	else if (type == DERIVED)
	{
		DerivedEntry* de = entry->type->structure;
		AliasListNode* cur = de->aliases;
		printf("%s", de->name);
		while (cur)
		{
			printf(", %s", cur->RUName);
			cur = cur->next;
		}
	}
	printf("\n");
}

void printTypeExpression(TypeLog* entrytype)
{
	TypeTag type = entrytype->entryType;

	if (type == INT)
		printf("int");
	else if (type == REAL)
		printf("real");
	else if (type == DERIVED)
	{
		printf("<");
		DerivedEntry* de = entrytype->structure;
		TypeInfoListNode* cur = de->list->head;

		printTypeExpression(cur->type);
		cur = cur->next;
		while (cur)
		{
			printf(", ");
			printTypeExpression(cur->type);

			cur = cur->next;
		}
		printf(">");
	}
}

void printVariableUsage(VariableEntry* entry)
{
	if (entry->usage == LOCAL)
		printf("local");
	else if (entry->usage == INPUT_PAR)
		printf("input parameter");
	else if (entry->usage == OUTPUT_PAR)
		printf("output parameter");
	printf("\n");
}

void printVariables(char* key, TrieEntry* entry)
{
	TypeLog* typelog = entry->ptr;

	if (typelog->entryType != VARIABLE)
		return;

	VariableEntry* varentry = typelog->structure;
	printf("Name - %s\n", varentry->name);
	printf("Scope - %s\n", local_func->name);

	printf("Type Name - ");
	printTypeName(varentry);

	printf("Type Expression - ");
	printTypeExpression(varentry->type);
	printf("\n");

	printf("Width - %d\n", varentry->type->width);
	printf("isGlobal - %s", varentry->isGlobal ? "Global\n" : "---\n");

	printf("Offset - %d\n", varentry->offset);

	printf("Variable Usage - ");
	printVariableUsage(varentry);
	printf("\n");
}

void printFunctions(char* key, TrieEntry* entry)
{
	TypeLog* typelog = entry->ptr;

	if (typelog->entryType != FUNCTION)
		return;

	local_func = (FuncEntry*)typelog->structure;
	iterateTrie(local_func->symbolTable, printVariables);
}

void printActivationRecSize(char* key, TrieEntry* entry)
{
	TypeLog* typelog = entry->ptr;

	if (typelog->entryType != FUNCTION)
		return;

	local_func = (FuncEntry*)typelog->structure;
	printf("%s - %d\n", local_func->name, local_func->activationRecordSize);
}

void printRecordDetails(char* key, TrieEntry* entry)
{
	TypeLog* typelog = entry->ptr;

	if (typelog->entryType != DERIVED)
		return;


	DerivedEntry* de = typelog->structure;
	printf("%s - ", key);
	printf("<");
	TypeInfoListNode* cur = de->list->head;

	printTypeExpression(cur->type);
	cur = cur->next;
	while (cur)
	{
		printf(", ");
		printTypeExpression(cur->type);

		cur = cur->next;
	}

	printf("> %d\n", typelog->width);
}