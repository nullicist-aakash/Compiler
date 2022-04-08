#include "toposort.h"
#include <stdlib.h>
int ind;
int cycle_detected;

void cycle_dfs(int** adj, int t, int* fin, int* vis)
{
	vis[t] = 1;
	for (int i = 0; i < adj[t][0]; i++)
	{
		if (vis[adj[t][i]] == 0)
			cycle_dfs(adj, adj[t][i], fin, vis);
		else
			if (fin[adj[t][i]] == 0)
				cycle_detected = 1;
	}
	fin[t] = 1;
}

void topo_dfs(int** adj, int t, int* arr, int* vis, int size)
{
	vis[t] = 1;
	
	for (int i = 0; i < size; i++)
	{
		if (vis[i] == 0 && adj[t][i] > 0)
			topo_dfs(adj, i, arr, vis, size);
	}

	arr[ind] = t;
	ind++;
}

int topologicalSort(int** adj, int* sortedList, int size)
{
	cycle_detected = 0;
	ind = 0;

	int* arr = calloc(size, sizeof(int));
	int* vis = calloc(size, sizeof(int));
	int* fin = calloc(size, sizeof(int));

	for (int i = 0; i < size; i++)
		vis[i] = fin[i] = 0;

	//check cycle
	for (int i = 0; i < size; i++)
		if (vis[i] == 0)
			cycle_dfs(adj, i, fin, vis);

	if (cycle_detected == 1)
	{
		free(vis);
		free(arr);
		free(fin);
		return -1;
	}

	//No cycle is detected, perform topo sort
	for (int i = 0; i < size; i++)
		vis[i] = 0;

	for (int i = 0; i < size; i++)
		if (vis[i] == 0)
			topo_dfs(adj, i, arr, vis, size);

	for (int i = 0; i < size; i++)
		sortedList[i] = arr[size - i - 1];

	for (int i = 0; i < size; i++)
		printf("%d ", arr[i]);
	printf("\n");

	for (int i = 0; i < size; i++)
		printf("%d ", sortedList[i]);
	printf("\n");

	free(vis);
	free(arr);
	free(fin);
	return 0;
}