/*
Bellman-Ford Algorithm for Single source shortest path
*/

#include<stdio.h>
#include<stdlib.h>
#include<project2.h>
#include<string.h>
#include<unistd.h>
#include<time.h>

#define SRC_VERTEX 0
#define INF 99999


int node_cnt;
int edge_cnt;
int insert_cnt=0;

/*
Bellman graph intialization based on nodes and edges...
*/
struct bellman_graph *create_graph(int nodes, int edges)
{
	struct bellman_graph *grph=(struct bellman_graph *)malloc( sizeof(struct bellman_graph) );	
	
	if(!grph)
	{
		
		printf("\n Mmeory allocation to graph failed...");
	}
	else
	{
		grph->no_of_nodes=nodes;
		grph->no_of_edges=edges;
		
		grph->edge_array=(struct edge_list *)malloc( edges * sizeof(struct edge_list) );
		
		if(!grph->edge_array)
		{
			printf("\n Mmeory allocation to grph->edge_array failed...");
			return NULL;
		}
	}
	
	return grph;
}

/*
Get edge info from graph file...
insert to graph adjacency list...
*/
struct bellman_graph *insert_edge(struct bellman_graph *grp)
{
	FILE *fp;
	char *temp[2], *src_token, *dst_token, str1[20];
	int i=0, len=256, rand_wet, k=0;
	int src, dest, line=0;
	
	bzero(str1,20);
		
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
		//printf("\n %s", temp[i]);
		line++;
		strncpy(str1,temp[i],strlen(temp[i]));
		
		if( isspace(str1[0]) )
		{
			/*File validation check....
			  used to count blank lines in file...							
			*/
			continue;
		}
		src_token=strtok(str1,"\t");
		dst_token=strtok(NULL,"\n");
		
		src=atol(src_token);
		dest=atol(dst_token);
		
		rand_wet= 1 + rand() % 10;
		
		if(k < grp->no_of_edges)
		{
			grp->edge_array[k].source=src;
			grp->edge_array[k].destin=dest;
			grp->edge_array[k].weight=rand_wet;
			k++;
		}		
	}
	fclose(fp);	
	return grp;
}

/*
To display adjacency list of graph...
*/

void print_graph(struct bellman_graph *grp)
{
	int i;

	printf("\n SRC \t DEST \t WET");	
	for(i=0; i< edge_cnt; i++)
	{
		printf("\n %d\t %d \t %d", grp->edge_array[i].source, grp->edge_array[i].destin, grp->edge_array[i].weight);
	}
	printf("\n");
}

/*
get number of nodes & edges from raw graph file...
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
Bellman ford algorithm....
*/
void bellman_ford(struct bellman_graph *bgr, int src)
{
	int i, j, u, v, wet;
	int vertex=bgr->no_of_nodes;
	int adges=bgr->no_of_edges;
	
	int *cost=(int *)malloc(vertex * sizeof(int));  //distance matrix...
	
	if(!cost)
	{
		printf("Error on cost[] memory allocation...");
		perror("malloc");
	}
	
	//Initialise cost matrix to infinity distance...
	
	for(i=0; i< vertex; i++)
	{
		cost[i]=INF;
	}
	
	//Source distance marked to 0...
	cost[src]=0;
	
	for(i=1; i<= vertex-1; i++)
	{
		for(j=0; j< adges; j++)
		{
			u=bgr->edge_array[j].source;
			v=bgr->edge_array[j].destin;
			wet=bgr->edge_array[j].weight;
			
			if (cost[u] != INF && cost[u] + wet < cost[v])
                cost[v] = cost[u] + wet;		
		}	
	}
	
	/*
		Check for negative weight cycle...
	*/	
	for (i = 0; i < adges-1; i++)
    {
        u = bgr->edge_array[i].source;
        v = bgr->edge_array[i].destin;
        wet = bgr->edge_array[i].weight;
        if (cost[u] != INF && cost[u] + wet < cost[v])
            printf("\n Ohh... Graph has Negative cycle is there...");
    }
	
	print_short_dist(cost,vertex);
}

int main()
{

	int i, j, ret;
	char *value, *token, str[100];
	bzero(str,100);
	
	struct bellman_graph *bgr;
	
	value=malloc(100);
	
	value=get_count_nodes(value);	
	strcpy(str,value);
	
	token=strtok(str,",");
	node_cnt=atoi(token);
	
	token=strtok(NULL,"\n");
	edge_cnt=atoi(token);
	free(value);

	printf("Bellman Ford Algorithm: Vertices=%d & Edges=%d \n", node_cnt, edge_cnt);	
	
	bgr=create_graph(node_cnt, edge_cnt);		
	
	bgr=insert_edge(bgr);
	
	struct timeval start, stop;
	print_graph(bgr);	
	gettimeofday(&start, NULL);
	bellman_ford(bgr, SRC_VERTEX);
	gettimeofday(&stop, NULL);
	
	
	printf("Bellman-Ford Running Time: %lu sec \n", stop.tv_sec - start.tv_sec );
	
	
	free(bgr);
		
	return 0;

}
