#include "AST.h"
#include <assert.h>

Token* copy_token(Token* input)
{
	assert(input != NULL);
	assert(input->length > 0);

	Token* out = calloc(1, sizeof(Token));
	out->type = input->type;
	out->lexeme = calloc(out->length, sizeof(char));
	strcpy(out->lexeme, input->lexeme);
	out->line_number = input->line_number;
	out->start_index = input->line_number;
	out->length = input->length;
	return out;
}

void allocateChildren(ASTNode* node, int count)
{
	node->childCount = count;
	node->children = calloc(count, sizeof(ASTNode*));
}

ASTNode* createLeafNode(TreeNode* leaf, TreeNode* parent)
{
	assert(leaf != NULL && leaf->isLeaf);

	ASTNode* node = calloc(1, sizeof(ASTNode));
	node->token = copy_token(leaf->token);
	node->symbol_index = leaf->symbol_index;
	node->isLeaf = 1;
	node->childCount = 0;
	node->parent = parent;
	return node;
}

ASTNode* performRecursion(TreeNode* input, TreeNode* parent, ASTNode* inherited)
{
	assert(input != NULL);

	if (input->isLeaf)
		return createLeafNode(input, parent);

	ASTNode* node = calloc(1, sizeof(ASTNode));
	node->symbol_index = input->symbol_index;
	node->isLeaf = 0;
	node->parent = parent;

	if (input->productionNumber == 0)
	{

	}
	// .....
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
		allocateChildren(node, 2);
		node->children[0] = performRecursion(input->children[2], input, NULL);
		node->children[1] = performRecursion(input->children[4], input, NULL);

		// assuming stmt is not nullable
		assert(node->children[1] != NULL);
		node->children[1]->sibling = performRecursion(input->children[5], input, NULL);
	}
	// ...
	else if (input->productionNumber == 60)
	{
		// <arithmeticExpression> ===> <term> <expPrime>
		// <arithmeticExpression>.treenode = <expPrime>.syn
		// <expPrime>.inh = <term>.treenode

		free(node);
		ASTNode* termNode = performRecursion(input->children[0], input, NULL);
		ASTNode* expPrime = performRecursion(input->children[1], input, termNode);
		return expPrime;
	}
	else if (input->productionNumber == 61)
	{
		// <expPrime> ===> <lowPrecedenceOperators> <term> <expPrime[1]>
		// <expPrime[1]>.inh = createTreeNode(<lowPrecedenceOperators>.data, <expPrime>.inh, <term>.treenode)
		// <expPrime>.syn = <expPrime[1]>.syn

		free(node);

		ASTNode* op = performRecursion(input->children[0], input, NULL);
		ASTNode* term = performRecursion(input->children[1], input, NULL);
		
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
	// ...
	else if (input->productionNumber == 94)
	{
		// <A> ===> TK_UNION
		// <A>.data = "union"

		node->token = copy_token(input->children[0]->token);
	}

	return node;
}

ASTNode* createTree(TreeNode* input)
{
	assert(input != NULL);

	return performRecursion(input, NULL, NULL);
}