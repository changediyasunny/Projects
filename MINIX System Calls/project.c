#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "project2.h"


#define MAX_MSGS 16
#define MAX_RECV 5
#define MAX_SEND 5
#define MAX_MESG_LEN 100


int register_sender()
{
	/*it registers as a sender to PM server*/
	int sid;
	int status;
	int regnum=1;

	printf("\n Enter the sender ID:");
	scanf("%d", &sid);

	status=sys_msearch(sid, regnum);
	if(status == 1)
	{
		printf("\nDuplicate: Sender already there...");
	}
	else if(status == 0)
	{
		printf("Sender registered status=%d", status);
	}
	return status;
}

void register_receiver(int flag)
{
	/*it registers as a sender to PM server*/
	int i;
	int status, recid, regnum=1;

	if(flag==0)
	{
		printf("\n Manually registering receiver !");
		printf("\n Enter receiver ID:");
		scanf("%d", &recid);

		//check for receiver present or not...
		status=sys_msearch(recid, regnum);
		if(status == 1)
		{
			//already registered....
			printf("\n Duplicate: Register already there...");
		}
		else if(status == 0)
			printf("\n receiver registered:");
	}
	else if(flag==1)
	{
		status=sys_msearch(4, regnum);
		status=sys_msearch(5, regnum);
	}
	else
		printf("\n REG-RECEIVE:- Something wrong in passing argument...");
}


void deposite_mesg()
{
    printf("\n----Send message to queue.---\n");

    int sendID;
    printf("\n Enter your Sender ID :");
    scanf("%d",&sendID);

    /*
     * add check for sender list....
     * */
    int check;
    check = sys_msearch(sendID, 0);
    if(check == -2)
    {
    	//Duplicate found....
    	printf("\n Duplicate Sender ID Found, Register New ID:");
    }
    else if(check == 1)
    {
    	// New Sender ID registered...
    	char recvID[50];
   	    bzero(recvID,50);
   	    /*
   	     * Only 3 receivers per message are allowed..
   	     * user can only send message to 3 receiver's at max...
   	     * */
   	    printf("\n Enter Receiver's ID for message:");
   	    scanf (" %[^\n]%*c", recvID);

   	    char msg[MAX_MESG_LEN];
   	    printf("\n Enter a message :");
    	scanf (" %[^\n]%*c", msg);

    	if(strlen(msg) > MAX_MESG_LEN)
    	{
    		printf("\n message length max reached...");
    	}
    	else
    	{
    		int status;
    		status = sys_mdeposite(sendID, recvID, msg);

    		if(status == 8)
    		{
    			printf("\n Mailbox Overflow....System Halted");
    			printf("\n Goto Admin calls to clear it...");
    		}
    		else if(status == 1)
    		{
    			printf("\n Status:Deposite_msg : %d\n", status);
    		}
    	}
    }
    else if( check == -1)
    {
    	printf("\n Sender not registered...Please register First");
    }
}

int snapshot()
{
	char mails[100], user_list[20];
	int ret;
	bzero(mails, 100);

	ret = sys_snapshot(mails, user_list);

	printf("\n Mailbox Messages:=%s", mails);
	printf("\n Mailbox User Avail:=%s", user_list);

	return ret;
}

void garbage_mailbox()
{
	int ret, val;

	printf("\n 1. Sender List delete, \n2.Receiver list delete");
	printf("\n Mailbox delete ");
	scanf("\n %d", &val);

	ret= sys_clear_mailbox(val);

	if(ret == 1)
		printf("\n Garbage Collected Sender...");
	else if(ret == 2)
		printf("\n Garbage Collected Receiver...");
	else if(ret == -1)
	{
		printf("\n All Mails Gone......");
	}
}

int recover_halt()
{
	int ret;
	char receiver_data[100];
	bzero(receiver_data, 100);

	ret= sys_recoverhalt(receiver_data);
	printf("Receiver-Message List=%s", receiver_data);
	return ret;
}

int main(int argc, char **argv)
{
	int input=-1;
	int tmp;

	tmp=mailbox_init();
	register_receiver(1);

	printf("\n welcome to Mailbox IPC=%d", tmp);

	while(1)
	{

		printf("\n-----Available Operations---\n");
		printf("1. Register Sender for Mailbox \n");
		printf("2. Register Receiver for Mailbox \n");
		printf("3. Deposit message \n");
		printf("4. SnapShot Mailbox \n");
		printf("5. Garbage Mailbox \n");
		printf("6. Mailbox Halt recover \n");
        printf("0. Exit the Program \n");
		printf("Enter your choice : ");
		scanf(" %d",&input);
        switch(input)
        {
        	case 1:
        			register_sender();
        			break;
        	case 2:
        			register_receiver(0);
		            break;
		    case 3:
		    		deposite_mesg();
		            break;
		    case 4:
		    		snapshot();
		    		break;
		    case 5:
		    		garbage_mailbox();
		    		break;
		    case 6:
		    		recover_halt();
		    		break;
		    case 0:
		            exit(0);
		            break;
		    default :
		            printf("\nIncorrect input. Try again. \n");
		            break;
	    }
	}
}
