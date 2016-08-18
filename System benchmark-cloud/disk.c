#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<sys/time.h>

#define RANDOM 1
#define SEQU 0


char filename[20] = "test_disk.txt";
int test_size = 1000;
int block_size;


/*

	Disk Sequential Read
*/

void *seq_read(void *arg)
{
	int i;
	FILE *fd;
	fd = fopen(filename, "r");
	
	for(i=0; i<block_size && fgetc(fd)!=EOF; i++);
	
	fclose(fd);
}

/*

	Disk Sequential Write
*/

void *seq_write()
{
	int j;
	FILE *fd;
	fd = fopen(filename, "w");
  
	for(j=0; j<block_size; j++)
	{
		fputc('A', fd);
	}
	fclose(fd);
}



/*

	Disk Random Read
*/

void *random_read()
{
	int i;
	int ram = 0;
	FILE *fd = fopen(filename, "r");
	
	if(test_size - block_size > 0)
		ram = rand() % (test_size - block_size);

	fseek(fd, ram, SEEK_SET);
	
	for(i=0; i<block_size && fgetc(fd)!=EOF; i++);
	
	fclose(fd);
}



/*

	Disk Random Write
*/


void *random_write()
{
	int ram = 0;
	int i;
	FILE *fd = fopen(filename, "w");
	
	if(test_size - block_size > 0)
		ram = rand() % (test_size - block_size);
		
	fseek(fd, ram, SEEK_SET);
	
	for(i=0;i<block_size;++i)
	{
		fputc('A', fd);
	}
	
	fclose(fd);
}


/*

	Used to manage already open or closed files
*/
int create_file(int flag)
{
	int i, j;
	FILE *fp;
	fp = fopen(filename, "w+");
	if(fp && flag == 1)   //read operations...
	{
		for(i=0; i<test_size; i++)
			fputs("s",fp);
	}
	else if(fp && flag == 0)   //write operations...
	{
		fclose(fp);
		return 1;
	}
	else 
	{
		printf("\n File creation/open failed.");
		return -1;
	}
	fclose(fp);
	return 1;
}


/*

	Display Disk throughput and latency on stdin
*/
void print_result(char *cmd, int is_random, int no_thread, int64_t total_size, int64_t duration)
{
	
	FILE *fp;
	
	printf("\ncmd, Type, block_size, thread, time/s, Speed");
	printf("\n%s, %d, %d, %d, %.3f /ms, %.9f Mb/s", cmd, is_random, block_size, no_thread, (float)duration/1000000, 
				((float)total_size * 1000000000/1024/1024/duration));
									
}


/*

	Disk thread creation, concurrency amangement module.
*/

void disk(char *operation, int access_type, int no_thread)
{

	int i, j, flag = 2;
	int64_t duration = 0;
	
	struct timespec start, end;
	
	//Dynamicall creation of thread on user input...
	pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t) * no_thread);
	
	
	//Specify Random/ SEQ access type
	
	if(access_type == RANDOM)
	{
	    if(strcmp(operation,"read") == 0)
	    {
		    for(i=0; i < no_thread; i++)
    	    {
    	    	flag = 1;
		        if( create_file(flag) < 0)
        		{
        			exit(EXIT_FAILURE);
        		}
        			
		        if( pthread_create(&thread[i], NULL, &random_read, &i) )
		        {
		        	printf("\n Error: pthread_create Error...");
					exit(EXIT_FAILURE);
		        }
      		}
    	}
    	else if(strcmp(operation, "write") == 0)
    	{
		    for(i=0; i < no_thread; i++)
    	    {
    	   		flag = 0;
		        if( create_file(flag) < 0)
        		{
        			printf("\n Unable to create file.");
        			exit(EXIT_FAILURE);
        		}
        			
		        if( pthread_create(&thread[i], NULL, &random_write, &i) )
		        {
		        	printf("\n Error: pthread_create Error...");
					exit(EXIT_FAILURE);
		        }
      		}
		}
  	}
	else if(access_type == SEQU)  //Sequential operation starts here....
	{
	    if(strcmp(operation,"read") == 0)
	    {
		    for(i=0; i < no_thread; i++)
    	    {
    	    	flag = 1;
		        if( create_file(flag) < 0)
        		{
        			printf("\n Unable to create file.");
        			exit(EXIT_FAILURE);
        		}
        			
		        if( pthread_create(&thread[i], NULL, &seq_read, &i) )
		        {
		        	printf("\n Error: pthread_create Error...");
					exit(EXIT_FAILURE);
		        }
      		}
    	}
    	else if(strcmp(operation, "write") == 0)
    	{
		    for(i=0; i < no_thread; i++)
    	    {
    	   		flag = 0;
		        if( create_file(flag) < 0)
        		{
        			printf("\n Unable to create file.");
        			exit(EXIT_FAILURE);
        		}
        			
		        if( pthread_create(&thread[i], NULL, &seq_write, &i) )
		        {
		        	printf("\n Error: pthread_create Error...");
					exit(EXIT_FAILURE);
		        }
      		}
		}
  	}

	//Throughput calculations
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
	
	for(i=0; i<no_thread; i++)
	{
		pthread_join(thread[i], NULL);
	}
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	
	//Latency calculations	
	duration = ( (int64_t)end.tv_nsec - (int64_t)start.tv_nsec);

  	int64_t total_size = (int64_t)(block_size) * no_thread;
  	
  	//Display result
  	print_result(operation,access_type,no_thread,total_size,duration);
}


int main(int argc, char *argv[])
{
	int i, j;
	
	if(argc != 5)
  	{
    	printf("\n Usuage: <read/write> <random/seq> <block_size> <n_thread>");
    	printf("\n");
    	exit(EXIT_FAILURE);
  	}

	srand(time(NULL));
  
	int access_type = strcmp(argv[2], "random") == 0? RANDOM:SEQU;
  	block_size = atoi(argv[3]);
  	int no_thread = atoi(argv[4]);
  	
	disk(argv[1], access_type, atoi(argv[4]));
	
	printf("\n");
	return 0;
}

