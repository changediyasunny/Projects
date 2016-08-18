#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include<math.h>
#include<sys/time.h>
#include<sys/resource.h>

#define size 200

struct int_mat_struct
{
	int a[size][size];
	int b[size][size];
	int result[size][size];
};

struct float_mat_struct
{
	float p[size][size];
	float q[size][size];
	float final[size][size];
};


void print_matrix(float temp[size][size])
{
	int i,j;
	
	for(i=0; i<size; i++)
	{
		for(j=0; j<size; j++)
		{
			printf("%.2f \t", (float)temp[i][j]);
		}
		printf("\n");
	}
}

int power(int x, int y)
{
	int i;
	long number = 1;
	
	for(i=0;i<y;i++)
	{
		number = number * x;
	}
	return number;
}


void *int_ops(void *arg)
{
	long i;	
	
	int j=2, k=5, count;
	struct int_mat_struct *mat = (struct int_mat_struct *)arg;
	
	for(i=0; i<size; i++)
	{
		for(j=0; j<size; j++)
    	{
      		mat->result[i][j] = 0;
      		for(k=0; k<size ; k++)
		    {
        		mat->result[i][j] += (mat->a[i][k] ) * ( mat->b[k][j] );
      		}
	    }
	}
}

void *float_ops(void *arg)
{
	float i, j, k;
	
	struct float_mat_struct *ft_mat = (struct float_mat_struct *)arg;
	
	for(i=0; i<size; i++)
	{
		for(j=0; j<size; j++)
    	{
      		ft_mat->final[(int)i][(int)j] = 0.0;
      		for(k=0; k<size ; k++)
		    {
        		ft_mat->final[(int)i][(int)j] += (ft_mat->p[(int)i][(int)k] ) * ( ft_mat->q[(int)k][(int)j] );
      		}
	    }
	}
}



void integer_matrix(int matrix[size][size])
{
	int i, j, k;
	
	for(i=0; i<size; i++)
	{
		for(j=0; j<size; j++)
		{
			matrix[i][j] = (rand() % 10);
		}	
	}

}

void float_matrix(float matrix[size][size])
{
	float i, j, k;
	
	for(i=0; i<size; i++)
	{
		for(j=0; j<size; j++)
		{
			matrix[(int)i][(int)j] = ( (float)(rand() % 10 )/1.3);
		}	
	}

}


void cpu_flops(int no_of_thread)
{
	int i, j, k, m;
	int test_case = 2;
	
	struct timespec start, end;
	
	struct int_mat_struct *mat = malloc(sizeof(struct int_mat_struct));
	
	struct float_mat_struct *ft_mat = malloc(sizeof(struct float_mat_struct));
	
	if(mat == NULL | ft_mat == NULL)
	{
		printf("matrix out of memory\n");
	    exit(EXIT_FAILURE);
	}
	
	int64_t total_ops = power(10, 9) * no_of_thread;
	
	pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t) * no_of_thread);
	
	//integer Operation test ....	
	
	
	for(m=0; m < test_case; m++)
	{
		int64_t duration = 0.0;	//stores the floating of double value		
		
		//initialize both matrices
		integer_matrix(mat->a);
		integer_matrix(mat->b);
				
		for(i=0; i<no_of_thread; i++)
		{	
			if( pthread_create(&thread[i], NULL, &int_ops, mat) )
			{
				printf("\n Error: pthread_create Error...");
				exit(EXIT_FAILURE);
			}			
		}
		
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
	
		for(i=0; i<no_of_thread; i++)
		{
			pthread_join(thread[i], NULL);
		}
			
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	
		duration = ( (int64_t)end.tv_nsec - (int64_t)start.tv_nsec);
	
		printf("\n Integer:");
		printf("\n%lld, %lld, %.6f", total_ops, duration/1000000, ((float)total_ops * 1000000/duration) );
	}
	
	
	//Floating point operations.....
	
	for(m=0; m < test_case; m++)
	{
		int64_t duration = 0.0;	//stores the floating of double value		
		
		//initialize both matrices
		float_matrix(ft_mat->p);
		float_matrix(ft_mat->q);

		for(i=0; i<no_of_thread; i++)
		{	
			if( pthread_create(&thread[i], NULL, &float_ops, ft_mat) )
			{
				printf("\n Error: pthread_create Error...");
				exit(EXIT_FAILURE);
			}			
		}
		
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
		
		for(i=0; i<no_of_thread; i++)
		{
			pthread_join(thread[i], NULL);
		}
	
		
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		
		duration = ( (int64_t)end.tv_nsec - (int64_t)start.tv_nsec);
	
		printf("\n Float:");
		printf("\n%lld, %lld, %.6f", total_ops, duration/1000000, ((float)total_ops * 1000000/duration) );
		
	}		
}


int main(int argc, char *argv[])
{
	
	if(argc != 2)
	{
		printf("\n usage: <no of threads>");
		exit(EXIT_FAILURE);
	}
	
	srand(time(NULL)); // to use rand() function
	printf("\n %d-%d", sizeof(long long), sizeof(int64_t));	
	int no_of_thread = atoi(argv[1]);
	cpu_flops(no_of_thread);
	printf("\n");	
	return 0;
}
