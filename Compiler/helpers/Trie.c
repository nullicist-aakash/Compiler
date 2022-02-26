#include "Trie.h"
#include <assert.h>

int getIndex(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c - 'A';

	if (c >= 'a' && c <= 'z')
		return 26 + (c - 'a');

	if (c >= '0' && c <= '9')
		return 52 + (c - '0');

	if (c == '_')
		return 62;

	if (c == '#')
		return 63;

	assert(0);
}

// returns 0 on success, -1 on error
TrieNode* getRef(Trie* t, char* key)
{
	int keyIndex = 0;
	int TrieIndex;
	TrieNode* traverse = t->root;

	while (key[keyIndex] != '\0')
	{
		TrieIndex = getIndex(key[keyIndex++]);

		if (!traverse->children[TrieIndex])
			traverse->children[TrieIndex] = createNode();

		traverse = traverse->children[TrieIndex];
	}

	return traverse;
}

// returns 1 on success, 0 on not found
int exists(Trie* t, char* key)
{
	return getVal(t, key) != NULL;
}

void* getVal(Trie* t, char* key)
{
	int keyIndex = 0;
	int TrieIndex;
	TrieNode* traverse = t->root;

	while (traverse && key[keyIndex] != '\0')
	{
		TrieIndex = getIndex(key[keyIndex++]);
		traverse = traverse->children[TrieIndex];
	}

	if (!traverse)
		return NULL;

	return traverse->value;
}

TrieNode* createNode()
{
	TrieNode* newNode = (TrieNode*)malloc(sizeof(TrieNode));
	
	newNode->value = NULL;

	for (int i = 0; i < TRIE_CHILD_COUNT; i++)
		newNode->children[i] = NULL;

	return newNode;
}