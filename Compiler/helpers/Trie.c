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
TrieNode* insert(Trie* t, char* key, void* val)
{	
	int keyIndex=0;
	int TrieIndex= getIndex(key[keyIndex]);
	TrieNode* traverse = t->root;

	while(key[keyIndex]!='\0')
	{
		if(!traverse->children[TrieIndex])
			traverse->children[TrieIndex]=getNode();
		
		traverse = traverse->children[TrieIndex];
		index = getIndex(key[keyIndex]);
		keyIndex++;
	}
	return traverse;
}


// returns 1 on success, 0 on not found
int search(Trie* t, char* key);
{
	if(get(t,key) == NULL)
		return 0
	return 1;
}

void* get(Trie* t, char* key)
{
	int keyIndex=0;
	int TrieIndex= getIndex(key[keyIndex]);
	TrieNode* traverse = t->root;

	while(key[keyIndex]!='\0')
    {
        TrieIndex = getIndex(key[keyIndex]);
 
        if (!traverse->children[TrieIndex])
            return 0;
 
        traverse = traverse->children[TrieIndex];
    }
	
	return traverse->value;
}

TrieNode* getNode()
{
	TrieNode *newNode = NULL;

	newNode =  (TrieNode* )malloc(sizeof(TrieNode));

	if (newNode)
    {
        int i;
 
        newNode->value = NULL;
 
        for (i = 0; i < TRIE_CHILD_COUNT; i++)
            newNode->children[i] = NULL;
    }

	return newNode;
 
}