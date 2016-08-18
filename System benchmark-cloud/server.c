#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include <netinet/in.h>
#include <netdb.h> 


#define MAX_DATAGRAM_SIZE 65507

int no_of_threads;
const char *port_no;
int numchars;

struct sockaddr_storage client_addr;
socklen_t client_addr_len;
	
char datagram[MAX_DATAGRAM_SIZE];
	

/*
	UDP server for Thread management 

*/
void *udp_server(void *arg)
{
	//int numchars;
	int sock = *(int *)arg;
	
	//datagram[numchars+1] = '\0';
		
	printf("\nServer Msg received bytes=%d", numchars);
		
	char ack[10] = "UDP-ACK";
	numchars = sendto(sock, ack, strlen(ack), 0, (struct sockaddr *)&client_addr, client_addr_len);
								
	if(numchars < 0)
	{
		printf("\n Error writing Socket.");
		perror("sendto");
		exit(EXIT_FAILURE);
	}
	         
} 

/*

	UDP module for connection phase and send/receive packets...
*/

void udp_prog(int no_of_threads)
{
	int socket_fd, getaddr_fd;

	struct addrinfo hints, *result;
	
	bzero(datagram, MAX_DATAGRAM_SIZE);
	
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* UDP Datagram socket */
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0; 			//any protocol
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
	
	getaddr_fd = getaddrinfo(NULL, port_no, &hints, &result);
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
	
	if( bind(socket_fd, result->ai_addr, result->ai_addrlen) == -1 )
	{
		printf("\n Server BIND() connect error.");
		exit(EXIT_FAILURE);
	}
	
	listen(socket_fd, 5);
	//Receiving of message starts here...
	
	for(; ; )
	{
		client_addr_len = sizeof(struct sockaddr_storage);
		
		numchars = recvfrom(socket_fd, datagram, MAX_DATAGRAM_SIZE, 0, 
									(struct sockaddr *)&client_addr, &client_addr_len);
		if(numchars < 0)
		{
			printf("\nIgnoring failed request.");
			continue;
		}
		
		pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t) * no_of_threads);
	
		if(thread == NULL)
		{
			printf("\n Thread memory allocation failed.");
			exit(EXIT_FAILURE);
		}
	
		int i;	
		for(i=0; i<no_of_threads; i++)
		{	
			if( pthread_create(&thread[i], NULL, &udp_server, &socket_fd) )
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
}


void *tcp_server(void *arg)
{
	int new_sock = *(int *)arg;	
	
		int numchar1 = read(new_sock, datagram, MAX_DATAGRAM_SIZE);
     	if (numchar1 < 0)
     	{
     		printf("\n ERROR reading from socket");
     		exit(EXIT_FAILURE);
     	} 
		//strcat(datagram, "\0");
		printf("\nTCP Server Msg size received=%d bytes", numchar1);
		
		char ack[10] = "TCP-ACK";
		numchar1 = sendto(new_sock, ack, strlen(ack), 0, 
								(struct sockaddr *)&client_addr, client_addr_len);
								
		if(numchar1 < 0)
		{
			printf("\n Server Error acknowledging Socket.");
			exit(EXIT_FAILURE);
		}	

}

void tcp_prog(int no_of_threads)
{
	int socket_fd, getaddr_fd;
	
	
	struct addrinfo hints, *result;
	
	memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP Datagram socket */
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0; 			//any protocol
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
	
	getaddr_fd = getaddrinfo(NULL, port_no, &hints, &result);
	if(getaddr_fd != 0)
	{
		printf("\n getaddrinfo failure.");
		exit(EXIT_FAILURE);
	}
	
	/*
		socket(domain of communication, type (stream/datagram), protocol)
		it returns file descriptor for further communication...
	*/
	
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0)
	{
		printf("\n Error opening Socket.");
		exit(EXIT_FAILURE);		
	}
	
	if( bind(socket_fd, result->ai_addr, result->ai_addrlen) == -1 )
	{
		printf("\n Server BIND() connect error.");
		exit(EXIT_FAILURE);
	}
	
	listen(socket_fd,5);
	
	//Receiving of message starts here...
	
	for(; ; )
	{
		client_addr_len = sizeof(struct sockaddr_storage);
		
		int new_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		if(new_fd < 0)
		{
			printf("\nServer accept request failed.");
			exit(EXIT_FAILURE);
		}
		
		pthread_t *thread = (pthread_t *) malloc(sizeof(pthread_t) * no_of_threads);
	
		if(thread == NULL)
		{
			printf("\n Thread memory allocation failed.");
			exit(EXIT_FAILURE);
		}
	
		int i;	
		for(i=0; i<no_of_threads; i++)
		{	
			if( pthread_create(&thread[i], NULL, &tcp_server, &new_fd) )
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
	
}


int main(int argc, char *argv[])
{
	
	port_no = argv[2];
	
	if (argc != 4)
	{
    	printf("\n Usuage: <tcp/udp> <port> <thread>");
    	exit(EXIT_FAILURE);
	}
	no_of_threads = atoi(argv[3]);
	
	if(strcmp(argv[1], "udp")==0)
	{
    	udp_prog(no_of_threads);
	}
	else
	{
		tcp_prog(no_of_threads);
	
	}
	printf("\n");
	return 0;

}
