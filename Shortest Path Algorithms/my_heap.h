#include<stdio.h>

// Structure to represent a min heap node
struct heapnode
{
	int vert;
	int dist;
};

struct heur
{
	int source;
	int destin;
	int h_score;
};

/*
ASTAR
*/


struct open_list_heapnode
{
	int gscore;
	int fscore;
	int vert;
};

// Structure to represent a min heap
struct minheap
{
	int heapsize;	 // Number of heap nodes present currently
	int length; // Capacity of min heap
	int *map;	 
	struct heapnode **block;
	struct open_list_heapnode **oblock;
	struct heur *h_arr;
};





