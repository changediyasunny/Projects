#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/time.h>

#define NO_OF_BLOCKS 1000
#define RANDOM 1
#define SEQU 0


unsigned long long block_size;
char *src, *dest;
int iter = 0;

/*
	Sequential Read + Write

*/

void *seq_read_write()
{
	int k;
	char temp, temp2 = 'a';
	
	for(k=0; k<NO_OF_BLOCKS; k++)
	{
		memcpy(&src[k], &dest[k], block_size);
		
		/*temp = dest[k];
		src[k] = temp2;*/
	}
}

/*
	Random Read + Write

*/

void *random_read_write()
{
	int i, ran = 0;
	char temp, temp2 = 'a';
	
	srand(time(NULL));
	ran = rand()%50;
	
	for(i=0; i< NO_OF_BLOCKS; i++)
	{
		
		memcpy(&src[ran], &dest[ran], block_size);
		
		/*temp = dest[ran];
		src[ran] = temp2;*/
	}
	
}

/*
	random Memory access handler...
*/

void memory_random(int access_type, int no_thread)
{

	int i;
	struct timeval t1, t2;
	struct timespec start, end;
	
	unsigned long long duration = 0.0;
		
	unsigned long long total_size = ((unsigned long long)(block_size) * (unsigned long long)(NO_OF_BLOCKS) * (unsigned long long)no_thread);
	
	
	//Dynamically create threads...
	pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t) * no_thread);
	
    for(i=0; i < no_thread; i++)
    {
        			
	    if( pthread_create(&thread[i], NULL, &random_read_write, NULL) )
	    {
		   	printf("\n Error: pthread_create Error...");
			exit(EXIT_FAILURE);
	    }
    }
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
	for(i=0; i<no_thread; i++)
	{
		pthread_join(thread[i], NULL);
	}

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	
	duration = ( (unsigned long long)end.tv_nsec - (unsigned long long)start.tv_nsec);
			
  	float size1 = (float) ((total_size)/1024.0/1024.0);
  	printf("\n %llu, %.9f", total_size, size1);
    printf("\n random R-W =%d, %.5f, %.5f", no_thread,(float)(duration/1000000.0), (float)(size1)*1000000000/(float)duration );
    
}


/*
	Memory sequential handler...

*/

void memory_seq(int access_type, int no_thread)
{

	int i;
	struct timeval t1, t2;
	struct timespec start, end;
	
	unsigned long long duration = 0.0;
		
	unsigned long long total_size = ((unsigned long long)(block_size) * (unsigned long long)(NO_OF_BLOCKS) * no_thread);
	
	
	//Dynamically create threads...
	pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t) * no_thread);
	
    for(i=0; i < no_thread; i++)
    {
        			
	    if( pthread_create(&thread[i], NULL, &seq_read_write, NULL) )
	    {
		   	printf("\n Error: pthread_create Error...");
			exit(EXIT_FAILURE);
	    }
    }
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
	for(i=0; i<no_thread; i++)
	{
		pthread_join(thread[i], NULL);
	}

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	
	duration = ( (unsigned long long)end.tv_nsec - (unsigned long long)start.tv_nsec);
			
  	float size1 = (float) ((total_size)/1024.0/1024.0);
  	printf("\n %llu, %.9f", total_size, size1);
    printf("\n random R-W =%d, %.5f, %.5f", no_thread,(float)(duration/1000000.0), (float)(size1)/(sizeof(long))*1000000000/(float)duration );
    
}

/*

	Memory sub-Module for Random/SEQ

*/
void memory(int access_type, int no_thread)
{

	if(access_type == RANDOM)
	{

		memory_random(access_type, no_thread);    	
  	}
	else 
	{	    
		memory_seq(access_type, no_thread);
  	}
}

int main(int argc, char *argv[])
{
	int i, j;
	
	src = malloc(sizeof(char) * 1024 * 1024 * 8);
	dest = 	malloc(sizeof(char) * 1024 * 1024 * 8);
	
	if(argc != 4)
  	{
    	printf("\n Usuage: <random/seq> <block_size> <n_thread>");
    	printf("\n");
    	exit(EXIT_FAILURE);
  	}
	
	int no_thread = atoi(argv[3]);  
	int access_type = strcmp(argv[1], "random") == 0? RANDOM:SEQU;
  	  	
  	block_size = atoi(argv[2]);
  	
	memory(access_type, atoi(argv[3]));
	
	printf("\n");
	return 0;
}

