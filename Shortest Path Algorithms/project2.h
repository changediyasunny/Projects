#include<stdio.h>

#define MAX_EDGE line_count
#define NEW_NODE (struct list_node *)malloc( sizeof(struct list_node) )
#define NEW_EDGENODE (struct edge_node *)malloc( sizeof(struct edge_node) )

/*=========== Dijkstra's Algorithm ================*/
/*
Node to hold dest & src vertex
*/
struct list_node
{
	int dest;
	int cost;
	struct list_node *next;
};

/*
vert_array of head...
*/
struct adj_list
{
	struct list_node *head;
};


struct graph
{
	int no_of_nodes;
	struct adj_list *vert_array;
};


/*=========== Bellman-Ford Algorithm ================*/
/*
	Bellman Ford Algorithm...
*/
struct edge_list
{
	int source;
	int destin;
	int weight;
};

struct bellman_graph
{
	int no_of_nodes;
	int no_of_edges;
	struct edge_list *edge_array;
};


