/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/
#include "AST.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int count_functions(TreeNode *input)
{
	int ct = 1;

	// first goto left child of program
	input = input->children[0];

	// while we have any child of current node, it means it will expand into function
	while (input->child_count != 0)
	{
		++ct;
		input = input->children[1];
	}

	return ct;
}

Token *copy_token(Token *input)
{
	assert(input != NULL);
	assert(input->length > 0);

	Token *out = calloc(1, sizeof(Token));
	out->type = input->type;
	out->lexeme = calloc(input->length + 1, sizeof(char));
	strcpy(out->lexeme, input->lexeme);
	out->line_number = input->line_number;
	out->start_index = input->line_number;
	out->length = input->length;
	return out;
}

void allocateChildren(ASTNode *node, int count)
{
	node->childCount = count;
	node->children = calloc(count, sizeof(ASTNode *));
}

ASTNode *createLeafNode(TreeNode *leaf, TreeNode *parent)
{
	assert(leaf != NULL && leaf->isLeaf);

	ASTNode *node = calloc(1, sizeof(ASTNode));
	node->sym_index = leaf->symbol_index;
	node->token = copy_token(leaf->token);
	node->isLeaf = 1;
	node->childCount = 0;
	return node;
}

ASTNode *performRecursion(TreeNode *input, TreeNode *parent, ASTNode *inherited)
{
	assert(input != NULL);

	if (input->isLeaf)
		return createLeafNode(input, parent);

	ASTNode *node = calloc(1, sizeof(ASTNode));
	node->isLeaf = 0;
	node->sym_index = input->symbol_index;

	if (input->productionNumber == 0)
	{
		//<program> ===> <otherFunctions> <mainFunction>
		//<program>.treenode = createTreeNode(<otherFunctions>.treenode, <mainFunction>.treenode);
		allocateChildren(node, 2);
		node->children[0] = performRecursion(input->children[0], input, NULL);
		node->children[1] = performRecursion(input->children[1], input, NULL);
	}
	else if (input->productionNumber == 1)
	{
		//<mainFunction> ===> TK_MAIN <stmts> TK_END
		//<mainFunction>.treenode = createTreeNode("main",<stmts>.treenode);
		node->token = copy_token(input->children[0]->token);
		allocateChildren(node, 3);
		node->children[2] = performRecursion(input->children[1], input, NULL);
		// TODO Check if this is necessary and correct
	}
	else if (input->productionNumber == 2)
	{
		//<otherFunctions> ===> <function> <otherFunctions[1]>
		//<otherFunctions>.treenode = createTreeNodeList(head = <function>.treenode, tail = <otherFunctions>.treenode);
		free(node);
		ASTNode *func = performRecursion(input->children[0], input, NULL);
		func->sibling = performRecursion(input->children[1], input, NULL);
		return func;
	}
	else if (input->productionNumber == 3)
	{
		//<otherFunctions> ===> eps
		//<otherFunctions>.treenode = NULL;
		free(node);
		return NULL;
	}
	else if (input->productionNumber == 4)
	{
		//<function> ===> TK_FUNID <input_par> <output_par> TK_SEM <stmts> TK_END
		//<function>.treenode = createTreeNode(TK_FUNID.value, <input_par>.treenode, <output_par>.treenode, <stmts>.treenode);
		allocateChildren(node, 3);
		node->token = copy_token(input->children[0]->token);
		node->children[0] = performRecursion(input->children[1], input, NULL);
		node->children[1] = performRecursion(input->children[2], input, NULL);
		node->children[2] = performRecursion(input->children[4], input, NULL);
	}
	else if (input->productionNumber == 5)
	{
		//<input_par> ===> TK_INPUT TK_PARAMETER TK_LIST TK_SQL <parameter_list> TK_SQR
		//<input_par>.treenode = <parameter_list>.treenode;
		free(node);
		return performRecursion(input->children[4], input, NULL);
	}
	else if (input->productionNumber == 6)
	{
		//<output_par> ===> TK_OUTPUT TK_PARAMETER TK_LIST TK_SQL <parameter_list> TK_SQR
		//<output_par>.treenode = <parameter_list>.treenode;
		free(node);
		return performRecursion(input->children[4], input, NULL);
	}
	else if (input->productionNumber == 7)
	{
		//<output_par> ===> eps
		//<output_par>.treenode = NULL;
		free(node);
		return NULL;
	}
	else if (input->productionNumber == 8)
	{
		//<parameter_list> ===> <dataType> TK_ID <remaining_list>
		//<parameter_list>.treenode = createTreeNodeList(head = createTreeNode(<dataType>.data, TK_ID.value), tail = <remaining_list>.treenode);
		node->token = copy_token(input->children[1]->token);	// Stores name of id
		node->type = performRecursion(input->children[0], input, NULL);
		node->sibling = performRecursion(input->children[2], input, NULL);
	}
	else if (input->productionNumber == 9)
	{
		//<dataType> ===> <primitiveDatatype>
		//<dataType>.treenode = <primitiveDatatype>.treenode;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 10)
	{
		//<dataType> ===> <constructedDatatype>
		//<dataType>.treenode = <constructedDatatype>.treenode;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 11)
	{
		//<primitiveDatatype> ===> TK_INT
		//<primitiveDatatype>.data = "int";
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 12)
	{
		//<primitiveDatatype> ===> TK_REAL
		//<primitiveDatatype>.data = "real"
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 13)
	{
		//<constructedDatatype> ===> TK_RECORD TK_RUID
		//<constructedDatatype>.data = TK_RUID.data;
		free(node);
		ASTNode *temp = performRecursion(input->children[0], input, NULL);
		temp->sibling = performRecursion(input->children[1], input, NULL);
		return temp;
	}
	else if (input->productionNumber == 14)
	{
		//<constructedDatatype> ===> TK_UNION TK_RUID
		//<constructedDatatype>.data = TK_RUID.data;
		free(node);
		ASTNode *temp = performRecursion(input->children[0], input, NULL);
		temp->sibling = performRecursion(input->children[1], input, NULL);
		return temp;
	}
	else if (input->productionNumber == 15)
	{
		//<constructedDatatype> == = > TK_RUID
		//<constructedDatatype>.data = TK_RUID.data;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 16)
	{
		//<remaining_list> ===> TK_COMMA <parameter_list>
		//<remaining_list>.treenode = <parameter_list>.treenode;
		free(node);
		return performRecursion(input->children[1], input, NULL);
	}
	else if (input->productionNumber == 17)
	{
		//<remaining_list> ===> eps
		//<remaining_list>.treenode = NULL;
		free(node);
		return NULL;
	}
	else if (input->productionNumber == 18)
	{
		//<stmts> ===> <typeDefinitions> <declarations> <otherStmts> <returnStmt>
		//<stmts>.treenode = createTreeNode(<typeDefinitions>.treenode, <declarations>.treenode,<otherStmts>.treenode, <returnStmt>.treenode);
		allocateChildren(node, 4);
		node->children[0] = performRecursion(input->children[0], input, NULL);
		node->children[1] = performRecursion(input->children[1], input, NULL);
		node->children[2] = performRecursion(input->children[2], input, NULL);
		node->children[3] = performRecursion(input->children[3], input, NULL);
	}
	else if (input->productionNumber == 19)
	{
		//<typeDefinitions> ===> <actualOrRedefined> <typeDefinitions>1
		//<typeDefinitions>.treenode = 	createTreeNodeList(head = <actualOrRedefined>.treenode, tail = <typeDefinitions>1.treenode);
		free(node);
		// allocateChildren(node, 2);
		ASTNode *temp = performRecursion(input->children[0], input, NULL);
		temp->sibling = performRecursion(input->children[1], input, NULL);
		return temp;
	}
	else if (input->productionNumber == 20)
	{
		//<typeDefinitions> ===> eps
		//<typeDefinitions>.treenode = NULL;
		free(node);
		return NULL;
	}
	else if (input->productionNumber == 21)
	{
		//<actualOrRedefined> ===> <typeDefinition>
		//<actualOrRedefined>.treenode = <typeDefinition>.treenode
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 22)
	{
		//<actualOrRedefined> ===> <definetypestmt>
		//<actualOrRedefined>.treenode = <definetypestmt>.treenode;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 23)
	{
		//<typeDefinition> ===> TK_RECORD TK_RUID <fieldDefinitions> TK_ENDRECORD
		//<typeDefinition>.treenode = createTreeNode(TK_RUID, <fieldDefinitions>.treenode);
		//<typeDefinition>.data = "record"
		node->token = copy_token(input->children[0]->token);
		allocateChildren(node, 2);
		node->children[0] = performRecursion(input->children[1], input, NULL);
		node->children[1] = performRecursion(input->children[2], input, NULL);
	}
	else if (input->productionNumber == 24)
	{
		//<typeDefinition> ===> TK_UNION TK_RUID <fieldDefinitions> TK_ENDUNION
		//<typeDefinition>.treenode = createTreeNode(TK_RUID, <fieldDefinitions>.treenode);
		//<typeDefinition>.data = "union"
		node->token = copy_token(input->children[0]->token);
		allocateChildren(node, 2);
		node->children[0] = performRecursion(input->children[1], input, NULL);
		node->children[1] = performRecursion(input->children[2], input, NULL);
	}
	else if (input->productionNumber == 25)
	{
		//<fieldDefinitions> ===> <fieldDefinition>1 <fieldDefinition>2 <moreFields>
		//<fieldDefinitions>.treenode = createTreeNodeList( head = createTreeNode(<fieldDefinition>1.treenode, <fieldDefinition>2.treenode), tail = <moreFieds>.treenode);
		free(node);
		ASTNode *first = performRecursion(input->children[0], input, NULL);
		first->sibling = performRecursion(input->children[1], input, NULL);
		first->sibling->sibling = performRecursion(input->children[2], input, NULL);
		return first;
	}
	else if (input->productionNumber == 26)
	{
		//<fieldDefinition> ===> TK_TYPE <fieldType> TK_COLON TK_FIELDID TK_SEM
		//<fieldDefinition>.treenode = createTreeNode(<fieldType>.data, TK_FIELDID.value);
		node->token = copy_token(input->children[3]->token);
		node->type = performRecursion(input->children[1], input, NULL);
	}
	else if (input->productionNumber == 27)
	{
		//<fieldType> ===> <primitiveDatatype>
		//<fieldType>.data = <primitiveDatatype>.data;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 28)
	{
		//<fieldType> ===> TK_RUID
		//<fieldType>.data = TK_RUID.data;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 29)
	{
		//<moreFields> ===> <fieldDefinition> <moreFields>
		//<moreFields>.treenode = createTreeNodeList(head = <fieldDefinition>.treenode, tail = moreFields.treenode);
		free(node);
		ASTNode *first = performRecursion(input->children[0], input, NULL);
		first->sibling = performRecursion(input->children[1], input, NULL);
		return first;
	}
	else if (input->productionNumber == 30)
	{
		//<moreFields> ===> eps
		//<moreFields>.treenode = NULL;
		free(node);
		return NULL;
	}
	else if (input->productionNumber == 31)
	{
		//<declarations> ===> <declaration> <declarations>1
		//<declarations>.treenode = createTreeNodeList(head = <declaration>.treenode, tail = <declarations>1.treenode);
		free(node);
		ASTNode *first = performRecursion(input->children[0], input, NULL);
		first->sibling = performRecursion(input->children[1], input, NULL);
		return first;
	}
	else if (input->productionNumber == 32)
	{
		//<declarations> ===> eps
		//<declarations>.treenode = NULL;
		free(node);
		return NULL;
	}
	else if (input->productionNumber == 33)
	{
		//<declaration> ===> TK_TYPE <dataType> TK_COLON TK_ID <global_or_not> TK_SEM
		//<declaration>.treenode = createTreeNode(<dataType>.data, TK_ID.value, <global_or_not>.isGlobal);
		node->type = performRecursion(input->children[1], input, NULL);
		node->token = copy_token(input->children[3]->token);
		ASTNode *isGlobal = performRecursion(input->children[4], input, NULL);
		node->isGlobal = isGlobal == NULL ? 0 : 1;
		if (isGlobal)
			free(isGlobal);
	}
	else if (input->productionNumber == 34)
	{
		//<global_or_not> ===> TK_COLON TK_GLOBAL
		//<global_or_not>.isGlobal = true;
		node->isGlobal = 1;
		return NULL;
	}
	else if (input->productionNumber == 35)
	{
		//<global_or_not> ===> eps
		//<global_or_not>.isGlobal = false;
		free(node);
		return NULL;
	}
	else if (input->productionNumber == 36)
	{
		//<otherStmts> ===> <stmt> <otherStmts>1
		//<otherStmts>.treenode = createTreeNodeList(head = <stmt>.treenode, tail = <otherStmts>1.treenode);
		free(node);
		ASTNode *first = performRecursion(input->children[0], input, NULL);
		first->sibling = performRecursion(input->children[1], input, NULL);
		return first;
	}
	else if (input->productionNumber == 37)
	{
		//<otherStmts> ===> eps
		//<otherStmts>.treenode = NULL;
		free(node);
		return NULL;
	}
	else if (input->productionNumber == 38)
	{
		//<stmt> ===> <assignmentStmt>
		//<stmt>.treenode = <assignmentStmt>.treenode;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 39)
	{
		//<stmt> ===> <iterativeStmt>
		//<stmt>.treenode = <iterativeStmt>.treenode;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 40)
	{
		//<stmt> ===> <conditionalStmt>
		//<stmt>.treenode = <conditionalStmt>.treenode;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 41)
	{
		//<stmt> ===> <ioStmt>
		//<stmt>.treenode = <ioStmt>.treenode
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 42)
	{
		//<stmt> ===> <funCallStmt>
		//<stmt>.treenode = <funCallStmt>.treenode;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 43)
	{
		//<assignmentStmt> ===> <singleOrRecId> TK_ASSIGNOP <arithmeticExpression> TK_SEM
		//<assignmentStmt>.treenode = createTreeNode(<singleOrRecId>.treenode, <arithmeticExpression>.treenode;
		node->token = copy_token(input->children[1]->token);
		allocateChildren(node, 2);
		node->children[0] = performRecursion(input->children[0], input, NULL);
		node->children[1] = performRecursion(input->children[2], input, NULL);
	}
	else if (input->productionNumber == 44)
	{
		//<oneExpansion> ===> TK_DOT TK_FIELDID
		//<oneExpansion>.treenode = createTreeNode(TK_FIELDID.value);
		allocateChildren(node, 2);
		node->children[0] = performRecursion(input->children[0], input, NULL);
		node->children[1] = performRecursion(input->children[1], input, NULL);
	}
	else if (input->productionNumber == 45)
	{
		//<moreExpansions> ===> <oneExpansion> <moreExpansions>1
		//<moreExpansions>.treenode = createTreeNodeList(head = <oneExpansion>.treenode, tail = <moreExpansions>1.treenode);
		free(node);
		ASTNode *oneExp = performRecursion(input->children[0], input, NULL);
		assert(oneExp->childCount == 2);
		ASTNode *dot = oneExp->children[0];
		ASTNode *id = oneExp->children[1];
		free(oneExp);

		allocateChildren(dot, 2);
		dot->children[0] = inherited;
		dot->children[1] = id;

		return performRecursion(input->children[1], input, dot);
	}
	else if (input->productionNumber == 46)
	{
		//<moreExpansions> ===> eps
		//<moreExpansions>.treeNode = NULL;
		free(node);
		return NULL;
	}
	else if (input->productionNumber == 47)
	{
		//<singleOrRecId> ===> TK_ID <option_single_constructed>
		//<singleOrRecId>.treenode = createTreeNode(TK_ID.value, <option_single_constructed>.treenode);
		free(node);
		ASTNode *idNode = performRecursion(input->children[0], input, NULL);
		ASTNode *constructed = performRecursion(input->children[1], input, idNode);
		return constructed;
	}
	else if (input->productionNumber == 48)
	{
		//<option_single_constructed> ===> eps
		//<option_single_constructed>.treeNode = NULL;
		free(node);
		return inherited;
	}
	else if (input->productionNumber == 49)
	{
		//<option_single_constructed> ===> <oneExpansion> <moreExpansions>
		//<option_single_constructed>.treenode = createTreeNodeList(head = <oneExpansion>.treenode, tail = <moreExpansions>1.treenode);
		free(node);
		ASTNode *oneExp = performRecursion(input->children[0], input, NULL);
		assert(oneExp->childCount == 2);
		ASTNode *dot = oneExp->children[0];
		ASTNode *id = oneExp->children[1];
		free(oneExp);

		allocateChildren(dot, 2);
		dot->children[0] = inherited;
		dot->children[1] = id;

		return performRecursion(input->children[1], input, dot);
	}
	else if (input->productionNumber == 50)
	{
		// <funCallStmt> ===> <outputParameters> TK_CALL TK_FUNID TK_WITH TK_PARAMETERS <inputParameters> TK_SEM
		// <funCallStmt>.treenode = createTreeNode(TK_FUNID.val, <outputParameters>.treenode, <inputParameters>.treenode)

		node->token = copy_token(input->children[2]->token);
		allocateChildren(node, 2);
		node->children[0] = performRecursion(input->children[0], input, NULL);
		node->children[1] = performRecursion(input->children[5], input, NULL);
	}
	else if (input->productionNumber == 51)
	{
		// <outputParameters> ===> TK_SQL <idList> TK_SQR TK_ASSIGNOP
		// <outputParameters>.treenode = createTreeNode("<--", <idList>.treenode)

		free(node);
		return performRecursion(input->children[1], input, NULL);
	}
	else if (input->productionNumber == 52)
	{
		// <outputParameters> ===> eps
		// <outputParameters>.treenode = nullptr

		free(node);
		return NULL;
	}
	else if (input->productionNumber == 53)
	{
		// <inputParameters> ===> TK_SQL <idList> TK_SQR
		// <inputParameters>.treenode = <idList>.treenode

		free(node);
		return performRecursion(input->children[1], input, NULL);
	}
	else if (input->productionNumber == 54)
	{
		// <iterativeStmt> ===> TK_WHILE TK_OP <booleanExpression> TK_CL <stmt> <otherStmts> TK_ENDWHILE
		// <iterativeStmt>.treenode = createTreeNode("while", <booleanExpressions>.treenode, createTreeNodeList(head = <stmt>.treenode, tail = <otherStmts>.treenode));

		node->token = copy_token(input->children[0]->token);
		allocateChildren(node, 3);
		node->children[0] = performRecursion(input->children[2], input, NULL);
		node->children[1] = performRecursion(input->children[4], input, NULL);
		node->children[1]->sibling = performRecursion(input->children[5], input, NULL);
	}
	else if (input->productionNumber == 55)
	{
		//<conditionalStmt> == = > TK_IF TK_OP<booleanExpression> TK_CL TK_THEN<stmt><otherStmts><elsePart>
		//<conditionalStmt>.treenode = createTreeNode("if", <booleanExpression>.treenode, createTreeNodeList(head = <stmt>.treenode, tail = <otherStmts>.treenode), <elsePart>.treenode)
		node->token = copy_token(input->children[0]->token);
		allocateChildren(node, 3);
		node->children[0] = performRecursion(input->children[2], input, NULL);
		node->children[1] = performRecursion(input->children[5], input, NULL);
		node->children[2] = performRecursion(input->children[7], input, NULL);
		node->children[1]->sibling = performRecursion(input->children[6], input, NULL);
	}
	else if (input->productionNumber == 56)
	{
		//<elsePart> ===> TK_ELSE <stmt> <otherStmts> TK_ENDIF
		//<elsePart>.treenode = createTreeNode("else", createTreeNodeList(head=<stmt>.treenode, tail=<otherStmts>.treenode)
		free(node);
		ASTNode *first = performRecursion(input->children[1], input, NULL);
		first->sibling = performRecursion(input->children[2], input, NULL);
		return first;
	}
	else if (input->productionNumber == 57)
	{
		//<elsePart> ===> TK_ENDIF
		//<elsePart>.treenode = NULL
		free(node);
		return NULL;
	}
	else if (input->productionNumber == 58)
	{
		//<ioStmt> == = > TK_READ TK_OP<var> TK_CL TK_SEM
		//<ioStmt>.treenode = createTreeNode("read", <var>.treenode)
		node->token = copy_token(input->children[0]->token);
		allocateChildren(node, 1);
		node->children[0] = performRecursion(input->children[2], input, NULL);
	}
	else if (input->productionNumber == 59)
	{
		//<ioStmt> == = > TK_WRITE TK_OP<var> TK_CL TK_SEM
		//<ioStmt>.treenode = createTreeNode("write", <var>.treenode)
		node->token = copy_token(input->children[0]->token);
		allocateChildren(node, 1);
		node->children[0] = performRecursion(input->children[2], input, NULL);
	}
	else if (input->productionNumber == 60)
	{
		// <arithmeticExpression> ===> <term> <expPrime>
		// <arithmeticExpression>.treenode = <expPrime>.syn
		// <expPrime>.inh = <term>.treenode

		free(node);
		ASTNode *termNode = performRecursion(input->children[0], input, NULL);
		ASTNode *expPrime = performRecursion(input->children[1], input, termNode);
		return expPrime;
	}
	else if (input->productionNumber == 61)
	{
		// <expPrime> ===> <lowPrecedenceOperators> <term> <expPrime[1]>
		// <expPrime[1]>.inh = createTreeNode(<lowPrecedenceOperators>.data, <expPrime>.inh, <term>.treenode)
		// <expPrime>.syn = <expPrime[1]>.syn

		free(node);

		ASTNode *op = performRecursion(input->children[0], input, NULL);
		ASTNode *term = performRecursion(input->children[1], input, NULL);

		allocateChildren(op, 2);
		op->children[0] = inherited;
		op->children[1] = term;

		return performRecursion(input->children[2], input, op);
	}
	else if (input->productionNumber == 62)
	{
		// <expPrime> ===> eps
		// <expPrime>.syn = <expPrime>.inh

		free(node);
		return inherited;
	}
	else if (input->productionNumber == 63)
	{
		//<term> ===> <factor> <termPrime>
		//<term>.treenode = <termPrime>.syn
		//<termPrime>.inh = <factor>.treenode
		free(node);
		ASTNode *factorNode = performRecursion(input->children[0], input, NULL);
		ASTNode *termPrime = performRecursion(input->children[1], input, factorNode);
		return termPrime;
	}
	else if (input->productionNumber == 64)
	{
		//<termPrime> ===> <highPrecedenceOperators> <factor> <termPrime[1]>
		//<termPrime[1]>.treenode = createTreeNode(<highPrecedenceOperators>.data, <termPrime>.inh, <factor>.treenode)
		//<termPrime>.syn = <termPrime[1]>.syn
		free(node);

		ASTNode *op = performRecursion(input->children[0], input, NULL);
		ASTNode *term = performRecursion(input->children[1], input, NULL);

		allocateChildren(op, 2);
		op->children[0] = inherited;
		op->children[1] = term;

		return performRecursion(input->children[2], input, op);
	}
	else if (input->productionNumber == 65)
	{
		//<termPrime> ===> eps
		//<termPrime>.syn = <termPrime>.inh
		free(node);
		return inherited;
	}
	else if (input->productionNumber == 66)
	{
		//<factor> ===> TK_OP <arithmeticExpression> TK_CL
		//<factor>.treenode = <arithmeticExpression>.treenode
		free(node);
		return performRecursion(input->children[1], input, NULL);
	}
	else if (input->productionNumber == 67)
	{
		//<factor> ===> <var>
		//<factor>.treenode = <var>.treenode
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 68)
	{
		//<highPrecedenceOperators> ===> TK_MUL
		//<highPrecedenceOperators>.data = "*";
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 69)
	{
		//<highPrecedenceOperators> ===> TK_DIV
		//<highPrecedenceOperators>.data = "/";
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 70)
	{
		//<lowPrecedenceOperators> ===> TK_PLUS
		//<lowPrecedenceOperators>.data = "+";
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 71)
	{
		//<lowPrecedenceOperators> ===> TK_MINUS
		//<lowPrecedenceOperators>.data = "-";
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 72)
	{
		//<booleanExpression> ===> TK_OP <booleanExpression>1 TK_CL <logicalOp> TK_OP <booleanExpression>2 TK_CL
		//<booleanExpression>.treenode = createTreeNode(<logicalOp>.data, <booleanExpression>1.treenode, <booleanExpression>2.treenode);
		free(node);
		ASTNode *op = performRecursion(input->children[3], input, NULL);
		allocateChildren(op, 2);
		op->children[0] = performRecursion(input->children[1], input, NULL);
		op->children[1] = performRecursion(input->children[5], input, NULL);
		return op;
	}
	else if (input->productionNumber == 73)
	{
		//<booleanExpression> ===> <var>1 <relationalOp> <var>2
		free(node);
		ASTNode *op = performRecursion(input->children[1], input, NULL);
		allocateChildren(op, 2);
		op->children[0] = performRecursion(input->children[0], input, NULL);
		op->children[1] = performRecursion(input->children[2], input, NULL);
		return op;
	}
	else if (input->productionNumber == 74)
	{
		//<booleanExpression> ===> TK_NOT TK_OP <booleanExpression> TK_CL
		//<booleanExpression>.treenode = createTreeNode("~", <booleanExpression>.treenode);
		free(node);
		ASTNode *op = performRecursion(input->children[0], input, NULL);
		allocateChildren(op, 1);
		op->children[0] = performRecursion(input->children[2], input, NULL);
		return op;
	}
	else if (input->productionNumber == 75)
	{
		//<var> ===> <singleOrRecId>
		//<var>.treenode =  <singleOrRecId>.treenode;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 76)
	{
		//<var> ===> TK_NUM
		//<var>.treenode = createTreeNode(TK_NUM.value);
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 77)
	{
		//<var> ===> TK_RNUM
		//<var>.treenode = createTreeNode(TK_RNUM.value);
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 78)
	{
		//<logicalOp> ===> TK_AND
		//<logicalOp>.data=”&&&”;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 79)
	{
		//<logicalOp> ===> TK_OR
		//<logicalOp>.data=”@@@”;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 80)
	{
		//<relationalOp> ===> TK_LT
		//<relationalOp>.data=”<”;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 81)
	{
		//<relationalOp> ===> TK_LE
		//<relationalOp>.data=”<=”;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 82)
	{
		//<relationalOp> ===> TK_EQ
		//<relationalOp>.data=”==”;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 83)
	{
		//<relationalOp> ===> TK_GT
		//<relationalOp>.data=”>”;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 84)
	{
		//<relationalOp> ===> TK_GE
		//<relationalOp>.data=”>=”;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 85)
	{
		//<relationalOp> ===> TK_NE
		//<relationalOp>.data=”!=”;
		free(node);
		return performRecursion(input->children[0], input, NULL);
	}
	else if (input->productionNumber == 86)
	{
		//<returnStmt> ===> TK_RETURN <optionalReturn> TK_SEM
		//<returnStmt>.treenode = <optionalReturn>.treenode;
		free(node);
		return performRecursion(input->children[1], input, NULL);
	}
	else if (input->productionNumber == 87)
	{
		//<optionalReturn> ===> TK_SQL <idList> TK_SQR
		//<optionalReturn>.treenode = <idList>.treenode;
		free(node);
		return performRecursion(input->children[1], input, NULL);
	}
	else if (input->productionNumber == 88)
	{
		//<optionalReturn> ===> eps
		//<optionalReturn>.treenode = NULL;
		free(node);
		return NULL;
	}
	else if (input->productionNumber == 89)
	{
		//<idList> ===> TK_ID <more_ids>
		//<idList>.treenode = createTreeNodeList(head = TK_ID.value, tail = <more_ids>.treenode);
		node->token = copy_token(input->children[0]->token);
		node->sibling = performRecursion(input->children[1], input, NULL);
	}
	else if (input->productionNumber == 90)
	{
		//<more_ids> ===> TK_COMMA <idList>
		//<more_ids>.treenode = <idList>.treenode;
		free(node);
		return performRecursion(input->children[1], input, NULL);
	}
	else if (input->productionNumber == 91)
	{
		//<more_ids> ===> eps
		//<more_ids>.treenode = NULL;
		free(node);
		return NULL;
	}
	else if (input->productionNumber == 92)
	{
		//<definetypestmt> ===> TK_DEFINETYPE <A> TK_RUID1 TK_AS TK_RUID2
		//<definetypestmt>.treenode = createTreeNode(<A>.data, TK_RUID1.value ,TK_RUID2.value)
		node->token = copy_token(input->children[0]->token);
		allocateChildren(node, 3);
		node->children[0] = performRecursion(input->children[1], input, NULL);
		node->children[1] = performRecursion(input->children[2], input, NULL);
		node->children[2] = performRecursion(input->children[4], input, NULL);
	}
	else if (input->productionNumber == 93)
	{
		// <A> ===> TK_RECORD
		// <A>.data = "record"

		node->token = copy_token(input->children[0]->token);
	}
	else if (input->productionNumber == 94)
	{
		// <A> ===> TK_UNION
		// <A>.data = "union"

		node->token = copy_token(input->children[0]->token);
	}

	return node;
}

void printTabs(int tabCount)
{
	while (tabCount--)
		printf("\t");
}

void print(ASTNode* node, int tab)
{
	if (node == NULL)
		return;

	printTabs(tab);
	printf("{ symbol: '%s', lexeme: '%s' }\n", 
		parserData->symbolType2symbolStr[node->sym_index], 
		node->token ? node->token->lexeme : "");

	for (int i = 0; i < node->childCount; ++i)
		print(node->children[i], tab + 1);

	print(node->sibling, tab);
}


ASTNode *createAST(TreeNode *input)
{
	assert(input != NULL);

	ASTNode* node = performRecursion(input, NULL, NULL);
	print(node, 0);

	return node;
}