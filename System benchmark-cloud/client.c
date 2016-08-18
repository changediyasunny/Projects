#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/time.h>
#include<time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include <netinet/in.h>
#include <netdb.h> 
#include<sys/resource.h>



int data_size;

int total_data_size;
unsigned long total_duration = 0;
char *hostname, *port_no;


//Printing result of TCP & UDP throughput + Latency
void print_result(int no_of_threads)
{
	
	printf("\n Total time by All Threads: %.5f", (float)total_duration);
	printf("\n Total Data Size Transferred: %d",total_data_size );
	printf("\n Speed Achieved: %.9f Mbps", ((float)total_data_size)*8*1000000000/1024.0/1024.0/(float)(total_duration) );
	
}


//Used to fill buffer for packet transfer.
void fill_buffer(char *buffer)
{
	int i;
	
	for(i=0; i<data_size; i++)
	{
	
		buffer[i] = 'A';
	}
	strncat(buffer,"\0", 1);
}


/*	UDP Client module program for connection establishment,
	send or receive packet from server
*/ 
void udp_prog(void *arguments)
{
	int socket_fd, getaddr_fd;
	int datagram_size, ack_size;
	
	unsigned long duration = 0;	
	
	struct timespec start, end;
	
	char datagram[data_size];
	bzero(datagram, data_size);
	
	//Filling buffer with random data...
	fill_buffer(datagram);	
	
		
	struct addrinfo hints, *result;
	
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* UDP Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0; 
	
	//Get server address info in structure...
	getaddr_fd = getaddrinfo(hostname, port_no, &hints, &result);
	if(getaddr_fd != 0)
	{
		printf("\n getaddrinfo failure.");
		exit(EXIT_FAILURE);
	}
	
	
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_fd < 0)
	{
		printf("\n Error opening Socket.");
		exit(EXIT_FAILURE);		
	}
	
	if( connect(socket_fd, result->ai_addr, result->ai_addrlen) == -1 )
	{
		printf("\n Client connect error.");
		exit(EXIT_FAILURE);
	}
	
	//Sending of message starts here...
	
	
	datagram_size = strlen(datagram);
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
	
	int numchars = write(socket_fd, datagram, data_size);
	if(numchars < 0)
	{
		printf("\nError sending to Socket.");
		exit(EXIT_FAILURE);
	}

	bzero(datagram, data_size);
	
	char ack_msg[20];
	
	//Get acknowledgement from server...
	numchars = read(socket_fd, ack_msg, data_size);	
	if(numchars < 0)
	{
		printf("\n Error reading Socket.");
		exit(EXIT_FAILURE);
	}
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	
	ack_msg[numchars+1] = '\0';
	
	printf("\n Client ACK Read=%s", ack_msg);
	ack_size = strlen(ack_msg);
	
	/*
		total time taken and bytes transferred...
	
	*/
	
	duration += ( (unsigned long)end.tv_nsec - (unsigned long)start.tv_nsec);
	
	total_data_size = datagram_size + ack_size;
	total_duration = (unsigned long)total_duration + (unsigned long)duration;
	
	printf("\n dur=%.5f ms, data_size=%d, Speed=%.5f", (float)duration/1000000.0, total_data_size, 
							(float)total_data_size *8*1000000000/1024.0/1024.0/(float)(total_duration));
	
	printf("\n ");
	
}


//UDP Client sub-module to create thread and Join for concurrency
void udp_client(int no_of_threads)
{
	int i, j;
	
	pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t) * no_of_threads);
	
	if(thread == NULL)
	{
		printf("\n Thread memory allocation failed.");
		exit(EXIT_FAILURE);
	}
	
	for(i=0; i<no_of_threads; i++)
	{	
		if( pthread_create(&thread[i], NULL, &udp_prog,(void *)i) )
		{
			printf("\n Error: pthread_create Error...");
			exit(EXIT_FAILURE);
		}			
	}
	
	for(i=0; i<no_of_threads; i++)
	{
		pthread_join(thread[i], NULL);
	}
	
	//print_result(no_of_threads);
}



/*	TCP Client module program for connection establishment,
	send or receive packet from server
*/ 

void tcp_prog(void *arguments)
{
	int socket_fd, getaddr_fd;
	int datagram_size, ack_size;
	
	unsigned long duration = 0;	
	
	struct timespec start, end;
	
	char datagram[data_size];
	bzero(datagram, data_size);
	
	//Fill buffer to be sent over TCP connection...
	fill_buffer(datagram);
	
	struct addrinfo hints, *result;
	
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0; 
	
	
	getaddr_fd = getaddrinfo(hostname, port_no, &hints, &result);
	if(getaddr_fd != 0)
	{
		printf("\n getaddrinfo failure.");
		exit(EXIT_FAILURE);
	}
	
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("\n Error opening Socket.");
		exit(EXIT_FAILURE);		
	}
	
	if( connect(socket_fd, result->ai_addr, result->ai_addrlen) == -1 )
	{
		printf("\n Client connect error.");
		exit(EXIT_FAILURE);
	}
	
	//Sending of message starts here...
	
	datagram_size = strlen(datagram);
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
	
	int numchars = write(socket_fd, datagram, data_size);
	if(numchars < 0)
	{
		printf("\nError sending to Socket.");
		exit(EXIT_FAILURE);
	}

	bzero(datagram, strlen(datagram));
	
	numchars = read(socket_fd, datagram, data_size);	
	if(numchars < 0)
	{
		printf("\n Error reading Socket.");
		exit(EXIT_FAILURE);
	}
	
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	
	
	printf("\n Client ACK Read=%s", datagram);
	ack_size = strlen(datagram);

	duration += ( (unsigned long)end.tv_nsec - (unsigned long)start.tv_nsec);
	
	total_data_size = (unsigned long)datagram_size + (unsigned long)ack_size;
	total_duration = (unsigned long)total_duration + (unsigned long)duration;

	printf("\n dur=%.5f ms, data_size=%d, Speed=%.5f", (float)duration/1000000.0, total_data_size, 
							(float)total_data_size *8*1000000000/1024.0/1024.0/(float)(total_duration));
	

	printf("\n ");
	
}


//TCP Client sub-module to create thread and Join for concurrency
void tcp_client(int no_of_threads)
{
	int i, j;
	
	pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t) * no_of_threads);
	
	if(thread == NULL)
	{
		printf("\n Thread memory allocation failed.");
		exit(EXIT_FAILURE);
	}
	
	for(i=0; i<no_of_threads; i++)
	{	
		if( pthread_create(&thread[i], NULL, &tcp_prog,(void *)i) )
		{
			printf("\n Error: pthread_create Error...");
			exit(EXIT_FAILURE);
		}			
	}
	
	for(i=0; i<no_of_threads; i++)
	{
		pthread_join(thread[i], NULL);
	}
	
	
}


//Main 
int main(int argc, char *argv[])
{

	hostname = argv[2];
	port_no = argv[3];
	data_size = atoi(argv[4]);
	
	if(data_size > 65507 )  //Buffer size for TCP & UDP...
	{
		printf("\n Max UDP packet size crossed.(65507 bytes)");
		exit(EXIT_FAILURE);
	}
	
	int no_of_threads = atoi(argv[5]);
		
	if (argc != 6)
	{
    	printf("\n Usuage: <tcp/udp> <host> <port> <buf-size> <n_thread>");
    	return 1;
	}

	if(strcmp(argv[1], "udp")==0)
	{
    	udp_client(no_of_threads);
	}
	else if( strcmp(argv[1], "tcp") == 0)
	{
    	tcp_client(no_of_threads);
	}
	else
	{
		printf("\n Usuage: <tcp/udp> <host> <port> <n_thread>");
    	return 1;
	}
	printf("\n");	
	return 0;

}
