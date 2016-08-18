/*
Dijkstra's Algorithm for Single source shortest path
*/

#include<stdio.h>
#include<stdlib.h>
#include<project2.h>
#include<my_heap.h>
#include<time.h>
#include<string.h>
#include<unistd.h>

#define MAX_WEIGHT 10
#define SRC_VERTEX 0
#define INF 9999999


int node_cnt;
int edge_cnt;
int insert_cnt=0;

/*
initialize a graph....
*/
struct graph *create_graph(int num)
{
	struct graph *grph=(struct graph *)malloc( sizeof(struct graph) );	
	
	if(!grph)
	{
		
		printf("\n Mmeory allocation to graph failed...");
	}
	else
	{
		//printf("\n Memory allocated to graph...");
		grph->no_of_nodes=num;
		
		//Array: used to hold each vertex...
		grph->vert_array=(struct adj_list *)malloc( num * sizeof(struct adj_list) );
		
		if(!grph->vert_array)
		{
			printf("\n Mmeory allocation to grph->vert_array failed...");
			return NULL;
		}
		int i;
		for(i=0; i<node_cnt; i++)
		{
			//making all head as null....
			grph->vert_array[i].head=NULL;
		}
		//printf("\n made head null=%d & node=%d", i, node_cnt);
	}
	
	return grph;
}

/*
Edge node attached to Array[u], creating edge u-->v
*/
struct list_node *create_node(int dest, int weight)
{
	struct list_node *tmp;
	
	tmp=NEW_NODE;
	
	if(!tmp)
	{
		printf("\n TMP list node allocation failed...");
		return NULL;
	}
	tmp->dest=dest;
	tmp->cost=weight;
	tmp->next=NULL;
	return tmp;
}

void insert_edge(struct graph *grp, long int src, long int dest, int weight)
{
	struct list_node *temp;
	
	temp=create_node(dest,weight);

	temp->next=grp->vert_array[src].head;
	grp->vert_array[src].head=temp;
	//insert_cnt++;
	//printf("\n Edge inserted:SRc=%ld & dest=%ld & WT=%d",src, dest, weight);
}

/*
Read the graph. and get SRC, DEST pair from graph file 
one line at a time...
*/

struct graph *get_edge(struct graph *grp)
{
	FILE *fp;
	char *temp[2], *src_token, *dst_token, str1[20];
	int i=0, len=256, blank=0, rand_wet;
	long int src, dest, line=0;
	
	bzero(str1,20);  //initialize str1 with all 0 bites...
		
	fp=fopen("road_data.txt","r");
	
	if(fp == NULL)
	{
		printf("\n Unable to open a file...");
		return NULL;

	}
	*temp=(char *)malloc( len );
	if(temp == NULL)
	{
		printf("\n Unable to allocate memory to TEMP");
		return NULL;
	
	}
	while(getline(&temp[i], &len, fp) != -1 )
	{
		line++;
		strncpy(str1,temp[i],strlen(temp[i]));
		
		if( isspace(str1[0]) )
		{
			/*File validation check....
			  used to count blank lines in file...							
			*/
			
			blank++;
			continue;
		}
		/*
		convert string to integer SRC,DEST pair from file...
		
		*/
		src_token=strtok(str1,"\t");
		dst_token=strtok(NULL,"\n");
		
		src=atol(src_token);
		dest=atol(dst_token);
		rand_wet= 1 + rand() % MAX_WEIGHT;
				
		insert_edge(grp, src, dest, rand_wet);		
	}

	fclose(fp);
	return grp;
}

/*
Used to print adjacency list graph generated from raw data file...
*/
void print_graph(struct graph *grp)
{
	int i;

	printf("\n SRC \t DEST \t WET");	
	for(i=0; i< node_cnt; i++)
	{
		struct list_node *temp=grp->vert_array[i].head;		
		while(temp)
		{
			printf("\n %ld\t %ld \t %d->", i, temp->dest, temp->cost);			
			temp=temp->next;
		}		
	}
	printf("\n");
}

/*

Used to get count of (nodes, edges) information from file...
later used in efficient memory allocation to adjacency list
and edge list node...
*/
char *get_count_nodes(char *temp1)
{
	char *temp[2], *src_token, *dst_token, str1[20];
	int i=0, len=256, line=0;
	int src_num, dst_num, max1=0, max2=0, max;
	char line_str[100], num_str[100], final_str[100];
	
    FILE *fp;
    
	bzero(str1,20);
	bzero(line_str,100);
	bzero(num_str,100);
	bzero(final_str, 100);
	
	*temp=(char *)malloc( len );
	
	fp=fopen("road_data.txt","r");	
	    
	while(getline(&temp[i], &len, fp) != -1 )
	{
		strncpy(str1,temp[i],strlen(temp[i]));
		
		if( isspace(str1[0]) )
		{
			//Invalid data in file, or space occured...
			continue;
		}
		src_token=strtok(str1,"\t");
		dst_token=strtok(NULL,"\n");		
		
		src_num=atol(src_token);
		dst_num=atol(dst_token);
		line++;
		if(src_num > max1)
			max1=src_num;
		
		if(dst_num > max2)
			max2=dst_num;
	
	}
	fclose(fp);
	
	if(max1 > max2)
		max=max1;
	else
		max=max2;
	
	sprintf(line_str,"%d", line);
	sprintf(num_str,"%d", max);
	
	strcat(num_str,",");
	strcat(num_str,line_str);
	temp1=num_str;
	
	return strdup(temp1);	
}

/*

Create a heap of size = no. of vertexes...
*/
struct minheap *create_heap(int num)
{
	struct minheap *minheap = (struct minheap *) malloc(sizeof(struct minheap));
	
	minheap->map = (int *)malloc(num * sizeof(int));
	minheap->heapsize = 0;
	minheap->length = num;
	minheap->block = (struct heapnode **) malloc(num * sizeof(struct heapnode *));

	return minheap;
}

/*
Node creation to insert into Heap...
node has information such as ( vertex & cost)
*/
struct heapnode *create_heapnode(int v, int dist)
{
	struct heapnode *heapnode=(struct heapnode *)malloc(sizeof(struct heapnode));
	heapnode->vert = v;
	heapnode->dist = dist;
	
	return heapnode;
}

/*
used to change location of heapnode into tree...
All node data in swapped...
*/
void swap_heap_node(struct heapnode **a, struct heapnode **b)
{
	struct heapnode *t = *a;
	*a = *b;
	*b = t;
}

/*
Used to update map[] array and cost of node in heap...
*/
void decreaskey(struct minheap *minheap, int src, int dist)
{
	/*
	Get the index of src in heap array...
	equivalent to get index from map[] array which will have known location
	of that vertex in binary heap... Here heap is array...
	*/
	
	int i = minheap->map[src];

	/*Update the distance of recent node into heap... */
	minheap->block[i]->dist = dist;


	/* Travel up while the complete tree is not hepified.
	   This is a O(Logn) loop
	   (i-1)/2 = parent node of "i" in a binary tree...
	*/
	while (i && minheap->block[i]->dist < minheap->block[(i - 1) / 2]->dist)
	{
		/*First update mapition into map[] array so later used to map 
		  correct index into binary heap....	
		*/
		minheap->map[minheap->block[i]->vert] = (i-1)/2;
		minheap->map[minheap->block[(i-1)/2]->vert] = i;
		
		// Swap this node with its parent
		swap_heap_node(&minheap->block[i], &minheap->block[(i - 1) / 2]);

		// move to parent
		i = (i - 1) / 2;  
	}
}

void heapify(struct minheap *minheap, int idx)
{
	int smallest, left, right;
	smallest = idx;
	left = 2 * idx + 1;
	right = 2 * idx + 2;

	if (left < minheap->heapsize && minheap->block[left]->dist < minheap->block[smallest]->dist )
		smallest = left;

	if (right < minheap->heapsize && minheap->block[right]->dist < minheap->block[smallest]->dist )
		smallest = right;

	if (smallest != idx)
	{
		// The nodes to be swapped in min heap
		struct heapnode *replace_node = minheap->block[smallest];
		struct heapnode *parent_node = minheap->block[idx];

		// Swap mapitions
		minheap->map[replace_node->vert] = idx;
		minheap->map[parent_node->vert] = smallest;

		// Swap nodes
		swap_heap_node(&minheap->block[smallest], &minheap->block[idx]);

		heapify(minheap, smallest);
	}
}

/*
Function to get root node = minimum node from heap...
*/
struct heapnode* extractmin(struct minheap* minheap)
{
	if (minheap->heapsize == 0)
		return NULL;

	// Store the root node which is min
	struct heapnode *root = minheap->block[0];

	// Replace root node with last node
	struct heapnode *last_node = minheap->block[minheap->heapsize - 1];
	minheap->block[0] = last_node;

	// Update mapition of last node
	minheap->map[root->vert] = minheap->heapsize-1;
	minheap->map[last_node->vert] = 0;

	// Reduce heap size and heapify root
	minheap->heapsize--;
	
	heapify(minheap, 0);

	return root;
}

/*
Validation check used to see heap size and node available in heap
*/
int is_in_heap(struct minheap *minheap, int v)
{
	if (minheap->map[v] < minheap->heapsize)
		return 1;
	return 0;
}

/*
Printing shortest distance from source node...
*/
void print_short_dist(int dist[], int n)
{	
	int i;
	
	printf("Shortest Distance:\n");
	printf("SRC \t DEST \t COST \n");
	for (i = 0; i < n; ++i)
		printf("%d \t %d \t %d\n", SRC_VERTEX, i, dist[i]);
}

/*
Dijkstra's algorithm...
*/
void dijkstra( struct graph *gr, int src)
{
	int i;
	int num_nodes=gr->no_of_nodes;
	int *cost=(int *)malloc(num_nodes * sizeof(int));  //distance matrix...
	
	if(!cost)
	{
		printf("\n Unable to allocate int array memory...");
		perror("malloc");
		exit(1);
	}
	
	struct minheap *minheap=create_heap(num_nodes);  //creates a whole heap
	
	for (i=0; i < num_nodes; i++)
	{
		//Initialize all heap array...
		cost[i]=INF;
		minheap->block[i] = create_heapnode(i, cost[i]);
		minheap->map[i] = i;
		
	}
	
	//Update source heap node and weight of source...
	minheap->block[src] = create_heapnode(src, cost[src]);
	minheap->map[src] = src;
	cost[src] = 0;
	
	decreaskey(minheap, src, cost[src]);

	minheap->heapsize=num_nodes;
	
	while(minheap->heapsize != 0 )
	{
		struct heapnode *heapnode = extractmin(minheap);
		
		int u = heapnode->vert; // Store the extracted vertex number

		// Traverse through all adjacent vertices of u (the extracted
		// vertex) and update their distance values
		struct list_node *target = gr->vert_array[u].head;
		while (target != NULL)
		{
			int v = target->dest;

			if (is_in_heap(minheap, v) && cost[u] != INF && target->cost + cost[u] < cost[v])
			{
				cost[v] = cost[u] + target->cost;
				// update distance value in min heap also
				decreaskey(minheap, v, cost[v]);
			}
			target = target->next;
		}	
	}

	for (i=0; i < num_nodes; i++)
	{
		;		
	}
	
	print_short_dist(cost, num_nodes);
}


int main()
{
	int i, j, ret;
	char *value, *token, str[100];
	bzero(str,100);
	
	struct graph *gr;
	
	value=malloc(100);
	
	value=get_count_nodes(value);	

	strcpy(str,value);
	
	token=strtok(str,",");
	node_cnt=atoi(token);
	
	token=strtok(NULL,"\n");
	edge_cnt=atoi(token);
	free(value);

	printf("Dijkstra's Algorithm: Vertices=%d & Edges=%d \n", node_cnt, edge_cnt);	
	gr=create_graph(node_cnt);		
	
	gr=get_edge(gr);
		
	struct timeval start, stop;
	print_graph(gr);	


	/*
	
	// To calculate time in Micro-Seconds....
	
	gettimeofday(&start, NULL);
	dijkstra(gr, SRC_VERTEX);
	gettimeofday(&stop, NULL);
	*/
	
	clock_t strt=clock();
	dijkstra(gr, SRC_VERTEX);
	clock_t end=clock();
	
	printf("Dijkstra's Running Time: %f", (double) (end-strt)/CLOCKS_PER_SEC );	
	
	//printf("Dijkstra's Running Time: %lu sec \n", stop.tv_sec - start.tv_sec );
	
	printf("\n");
	free(gr);
		
	return 0;
}













