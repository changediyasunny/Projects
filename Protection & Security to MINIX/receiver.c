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

	int recvID, chk2, mailbox_id, chk1;
	char msg[100], name[10];

	bzero(msg, 100);

	printf("\nEnter your Receiver ID :");
	scanf("%d",&recvID);


    int mtype;
    printf("\n Type of mailbox to retrieve message:(1.Public/0.Secure):");
    scanf("%d", &mtype);

    if(mtype == PUBLIC)
    {

    	strcpy(name, "public1");

    	mailbox_id=sys_mlist_search_name(name,-1,SEARCH,-3,MID);
    	if(mailbox_id == -1)
    	{
    		printf("\n No such mailbox there...");
    	}
    	else
    	{
        	chk2=sys_deny_access(recvID,mailbox_id,RECV_BIT,PUBCHK);

        	if(chk2 == 1 || chk2 == -1)
        	{
        		//Have access OR registered...
        		int status;
        		status = sys_mreceive(recvID, msg, mailbox_id);
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
        	{
        		//NO Access
        		printf("\n Receiver don't have access to PUBLIC mailbox:%d", mailbox_id);
        	}
    	}
    }
    else if(mtype == SECURE)
    {
        printf("\nEnter name of Mailbox to retrieve message:");
        scanf("%s", name);

        mailbox_id=sys_mlist_search_name(name,-1,SEARCH,-3, MID);
        printf("\n Mailbox_id for that name=%d", mailbox_id);

        if(mailbox_id == -1)
        {
        	//No Such mailbox Exists...
        	printf("\n No Such Mailbox Exists for receiver:");
        }
        else
        {

        	chk1=sys_msearch(recvID, SEARCH, mailbox_id);

        	if(chk1 == mailbox_id)
        	{

       			chk2=sys_deny_access(recvID,mailbox_id,RECV_BIT,CHKING);
        		if(chk2 == 1)
        		{
            		printf("\n Receiver pass all validation stages ! Now Receive.");
            		int status;
            		status = sys_mreceive(recvID, msg, mailbox_id);
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
        		{
        			printf("\n RECV Bit is OFF...Do not have access");
        		}

        	}
        	else
        		printf("\n No Such receiver for that Mailbox_ID=%d...", mailbox_id);
        }
    }

}

int main(int argc, char **argv)
{
	int input=-1;
	int tmp;
	receive_mesg();

	return 0;

}
