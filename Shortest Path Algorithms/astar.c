/*
 	ASTAR Algorithm for Single source shortest path
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


#define OPEN_LIST_NODE (struct open_list_heapnode **) malloc(num * sizeof(struct open_list_heapnode *));

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
		grph->no_of_nodes=num;
		
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
	
	bzero(str1,20);   //initialize str1 with all 0 bites...
		
	fp=fopen("roadNet-TX.txt","r");
	
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
		//printf("\n %s", temp[i]);
		line++;
		strncpy(str1,temp[i],strlen(temp[i]));
		
		if( isspace(str1[0]) )
		{
			blank++;
			/*File validation check....
			  used to count blank lines in file...							
			*/
			continue;
		}
		src_token=strtok(str1,"\t");
		dst_token=strtok(NULL,"\n");
		
		/*
		convert string to integer SRC,DEST pair from file...		
		*/
		
		src=atol(src_token);
		dest=atol(dst_token);
		
		rand_wet= 1 + rand() % 10;
				
		insert_edge(grp, src, dest, rand_wet);			
	}
	fclose(fp);
	return grp;
}


/*
To display graph adjacency list...
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
	
	fp=fopen("roadNet-TX.txt","r");	
	    
	while(getline(&temp[i], &len, fp) != -1 )
	{

		strncpy(str1,temp[i],strlen(temp[i]));
		
		if( isspace(str1[0]) )
		{
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
	minheap->oblock = OPEN_LIST_NODE;

	return minheap;
}

/*
The OPEN_SET[] array in A* algorithm is implemented as heap node...
saving memory space...
*/
struct open_list_heapnode *create_heapnode(int v, int g_dist, int f_dist )
{
	struct open_list_heapnode *heapnode=(struct open_list_heapnode *)malloc(sizeof(struct open_list_heapnode));
	heapnode->vert = v;
	heapnode->gscore=g_dist;
	heapnode->fscore=f_dist;
//	heapnode->dist = dist;
	
	return heapnode;
}

/*
Used to change node data into Binary heap...
*/
void swap_heap_node(struct open_list_heapnode **a, struct open_list_heapnode **b)
{
	struct open_list_heapnode *t = *a;
	*a = *b;
	*b = t;
}


/*
Used to update map[] array and cost of node in heap...
*/
void decreaskey(struct minheap *minheap, int src, int g_dist, int f_dist)
{
	int ret=0;

	/*
	Get the index of src in heap array...
	equivalent to get index from map[] array which will have known location
	of that vertex in binary heap... Here heap is array...
	*/
	int i = minheap->map[src];


	/*
	Heap is organized based on min F-Score value at parent node...
	*/
	if(g_dist <= minheap->oblock[i]->gscore)
	{
		minheap->oblock[i]->gscore = g_dist;
		minheap->oblock[i]->fscore = f_dist;
	}

	/* Travel up while the complete tree is not hepified.
	   This is a O(Logn) loop
	   (i-1)/2 = parent node of "i" in a binary tree...
	 
	   Heap node stores both F-Score & G-Score of search node
	*/
	while (i && minheap->oblock[i]->fscore < minheap->oblock[(i - 1) / 2]->fscore)
	{

		/*First update mapition into map[] array so later used to map 
		  correct index into binary heap....	
		*/

		minheap->map[minheap->oblock[i]->vert] = (i-1)/2;
		minheap->map[minheap->oblock[(i-1)/2]->vert] = i;
		swap_heap_node(&minheap->oblock[i], &minheap->oblock[(i - 1) / 2]);

		// move to parent index and start executing loop again...
		i = (i - 1) / 2;  
	}
}

/*
Heapify Procedure...
*/
void heapify(struct minheap *minheap, int idx)
{
	int smallest, left, right;
	smallest = idx;
	left = 2 * idx + 1;
	right = 2 * idx + 2;

	if (left < minheap->heapsize && minheap->oblock[left]->fscore < minheap->oblock[smallest]->fscore )
		smallest = left;

	if (right < minheap->heapsize && minheap->oblock[right]->fscore < minheap->oblock[smallest]->fscore )
		smallest = right;

	if (smallest != idx)
	{
		// The nodes to be swapped in min heap
		struct open_list_heapnode *smallestNode = minheap->oblock[smallest];
		struct open_list_heapnode *idxNode = minheap->oblock[idx];

		// Swap mapitions
		minheap->map[smallestNode->vert] = idx;
		minheap->map[idxNode->vert] = smallest;

		// Swap nodes
		swap_heap_node(&minheap->oblock[smallest], &minheap->oblock[idx]);

		heapify(minheap, smallest);
	}
}

/*
 OPEN-SET[] is used to extract minimum node from heap
*/
struct open_list_heapnode* extractmin(struct minheap *minheap)
{
	if (minheap->heapsize == 0)
		return NULL;

	// Store the root node
	struct open_list_heapnode *root = minheap->oblock[0];

	// Replace root node with last node
	struct open_list_heapnode *lastNode = minheap->oblock[minheap->heapsize - 1];
	minheap->oblock[0] = lastNode;

	// Update mapition of last node
	minheap->map[root->vert] = minheap->heapsize-1;
	minheap->map[lastNode->vert] = 0;

	// Reduce heap size and heapify root
	minheap->heapsize--;
	
	heapify(minheap, 0);

	return root;
}

int is_in_heap(struct minheap *minheap, int v)
{
	if (minheap->map[v] < minheap->heapsize)
		return 1;
	
	return 0;
}

/*
Print shortest distance...
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
ASTAR Algorithm...
*/
void astar( struct graph *gr, int src)
{
	int i, g_score, f_score, h_score=7, ret;
	int num_nodes=gr->no_of_nodes;

	int *g_cost=(int *)malloc(num_nodes * sizeof(int));       //G-COST[] matrix...
	int *f_cost=(int *)malloc(num_nodes * sizeof(int));      //F-COST[] matrix...
	int *closed_set=(int *)malloc(num_nodes * sizeof(int)); //Closed-SET[] matrix...
	
	
	if(!g_cost || !f_cost)
	{
		printf("\n Unable to allocate int array memory...");
		perror("malloc");
		exit(1);
	}
	
	struct minheap *minheap=create_heap(num_nodes);  //creates a whole heap
	
	/*
	Initialize gscore, fscore and open-set heap array...
	*/
	for (i=0; i < num_nodes; i++)
	{
		g_cost[i]=INF;
		f_cost[i]=INF;
		minheap->oblock[i] = create_heapnode(i, g_cost[i], f_cost[i]);
		minheap->map[i] = i;
		closed_set[i]=INF;		
	}
	
	/*
	For source node....
	G-Score is initialised, F-Score set to INF and 
	Heuristic function is set manually	
	*/
	
	g_score=0;
	f_score=g_score + h_score;

	g_cost[src]=g_score;
	f_cost[src]=f_score;
		
	minheap->oblock[src] = create_heapnode(src,g_score,f_score);
	minheap->map[src] = src;
	
	
	decreaskey(minheap, src, g_cost[src], f_cost[src]);

	minheap->heapsize=num_nodes;

	//printf("\n PLease go to loop...uu good boy....");
	
	while(minheap->heapsize != 0 )
	{
		//printf("\n Heyeyyey..m in the loopppp");
		
		struct open_list_heapnode *heapnode = extractmin(minheap);
		
		int u = heapnode->vert; // Store the extracted vertex number
		
		/*
		Already visited nodes kept into Closed-Set to avoid 
		traversing multiple times...
		*/	
		
		closed_set[u]=u;
		
		// Traverse through all adjacent vertices of u (the extracted
		// vertex) and update their distance values
		
		struct list_node *target = gr->vert_array[u].head;
		
		while (target != NULL)
		{
			int v = target->dest;
	
			if(v == closed_set[v] )
			{
				//compare to closed set...
				target = target->next;
				continue;
				
			}
		
			int temp_gscore;
		
			if (is_in_heap(minheap, v) && g_cost[u] != INF && target->cost + g_cost[u] < g_cost[v])
			{
				temp_gscore=g_cost[u] + target->cost;
				g_cost[v]= temp_gscore;
				f_cost[v]=g_cost[v]+3;
							
				// update distance value in min heap also
				decreaskey(minheap, v, temp_gscore, f_cost[v]);
							
			}			
			target = target->next;
		}	
	}
	//print_short_dist(g_cost, num_nodes);
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

	printf("Astar Algorithm: Vertices=%d & Edges=%d \n", node_cnt, edge_cnt);	
	gr=create_graph(node_cnt);		
	
	gr=get_edge(gr);
			
	//print_graph(gr);	
	struct timeval start, stop;
	//print_graph(gr);
	/*
	gettimeofday(&start, NULL);
	astar(gr, SRC_VERTEX);
	gettimeofday(&stop, NULL);
	*/
	
	clock_t strt=clock();
	astar(gr, SRC_VERTEX);
	clock_t end=clock();
	
	printf("Astar Running Time: %f sec \n", (double) (end-strt)/CLOCKS_PER_SEC );
	
	//printf("Astar Running Time: %.00lu sec \n", stop.tv_sec - start.tv_sec );
	
	
	free(gr);
			
	return 0;
}













