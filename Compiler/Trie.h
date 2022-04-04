/***************************************
				GROUP-08
  Yash Bansal			-   2019A7PS0484P
  Sourabh S Yelluru		-   2018B3A70815P
  Nihir Agarwal			-   2018B4A70701P
  Aakash				-   2018B4A70887P
*****************************************/
#pragma once
#define TRIE_CHILD_COUNT 64

typedef union TrieEntry {
	void* ptr;
	int value;
} TrieEntry;

typedef struct TrieNode
{
	TrieEntry entry;
	struct TrieNode* children[TRIE_CHILD_COUNT];
} TrieNode;


typedef struct Trie{
	TrieNode* root;
} Trie;


TrieNode* trie_getRef(Trie* t, char* key);

// returns 1 on success, 0 on not found
int trie_exists(Trie* t, char* key);

TrieEntry trie_getVal(Trie* t, char* key);