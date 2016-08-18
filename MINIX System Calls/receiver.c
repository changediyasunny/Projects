#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "project2.h"


#define MAX_MSGS 16
#define MAX_RECV 5
#define MAX_SEND 5
#define MAX_MESG_LEN 100


void receive_mesg()
{
    printf("\n----receive message from queue.---\n");
    int recvID;
    char msg[100];

    bzero(msg, 100);

    printf("\nEnter your Receiver ID :");
    scanf("%d",&recvID);

    // ADD:- check for receiver_list...
    int check;
    check = sys_msearch(recvID, 0);
    if(check == 1)
    {
        int status;
        status = sys_mreceive(recvID, msg);
        if(status == -1)
        {
        	printf("\n Mailbox Underflow...");
        }
        else if(status > 0)
        {
        	printf("\n retrieved message is= : %s\n", msg);
        }
        else if(status == 0)
        	printf("\n No Message There...");

    }
    else
    	printf("\n Error on Registering Receiver...");
}

int main(int argc, char **argv)
{
	int input=-1;
	int tmp;
	receive_mesg();

	return 0;

}
