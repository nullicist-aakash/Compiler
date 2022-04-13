/*rules
 isLeaf = true iff current node is leaf
 symbol_index = current non terminal index
 parent stores parent node and is NULL if current node is root
 token stores the corresponding token
 type != NULL iff current node stores the identifier
 children stores children
 sibling pointed to right element in a list
*/

#include "parserDef.h"

typedef struct ASTNode
{
	int isLeaf;
	int isGlobal;
	int sym_index;

	Token *token;
	struct ASTNode *type;
	struct TypeLog* derived_type;
	int childCount;
	struct ASTNode **children;
	struct ASTNode *sibling;
} ASTNode;