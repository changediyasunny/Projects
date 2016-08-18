#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "mailboxq.h"
#include<minix/endpoint.h>
#include<minix/com.h>
#include <minix/type.h>
#include "pm.h"
#include <unistd.h>
#include <minix/callnr.h>



void mail_init()
{
	printf("\n SYSCALL: mail_init...into mailboxq.c file");

	memset( mailbox, 0, sizeof(mailbox));
	memset( sender_list,0, sizeof(sender_list) );
	memset( recv_list,0, sizeof(recv_list) );
	memset(user_list,0,sizeof(user_list));

	mail_cnt=0;
	no_of_sender=0;
	no_of_recv=0;
	total_user=0;
}

int add_mail_list(int sid, int regnum)
{
	int found=0, i, mark=0;
	printf("\n mailboxq.c file: into add_mail_list");

	// Register here...
	for(i=0; i<total_user; i++)
	{
		if(user_list[i] == sid)
		{
			found=1;
			mark=1;
			printf("\n Sender/Receiver already there...");
		}
	}
	if(regnum == 1 && found == 0 && total_user < MAX_USER)
	{
		// Then register the sender...
		user_list[total_user] = sid;
		printf("\n User Registered...%d", user_list[total_user]);
		total_user++;

	}
	else if( total_user>= MAX_USER)
	{
		printf("\n User limit reached...");
		printf("\n total User registered...=%d", total_user);
	}
	else if(found == 1 && regnum == 1)
	{
		// Case of Duplication...
		return -2;
	}
	else if(regnum == 0 && found == 0)
	{
		// Case of search fail..Sender does not exists..
		return -1;
	}
	return found;
}

int add_sender_list(int sid, int regnum)
{
	int found=0, i;
	printf("\n mailboxq.c file: into add_sender list");

	for(i=0; i<no_of_sender; i++)
	{
		if(sender_list[i] == sid)
		{
			// if Duplicate sender id found...then
			found=1;
			if(regnum == 0)  // check for Searching...
			{
				return 0;  // trying to search & found...
			}
		}
	}
	/*
	 * regnum =1 indicates new register for sender
	 * Found = 0 indicates sender not available
	 * */
	if(regnum == 1 && found == 0 && no_of_sender < MAX_SEND)
	{
		// Then register the sender...
		sender_list[no_of_sender] = sid;
		no_of_sender++;

	}
	else if( no_of_sender>= MAX_SEND)
	{
		printf("\n sender limit reached...");
		printf("\n total sender registered...=%d", no_of_sender);
	}
	else if(found == 1 && regnum == 1)
	{
		// Case of Duplication...
		return 1;
	}
	else if(regnum == 0 && found == 0)
	{
		// Case of search fail..Sender does not exists..
		return -1;
	}

	return found;
}

int add_recv_list(int rid, int regnum)
{
	int found=0, i;
	printf("\n mailboxq.c file: into add_receiver list");

	for(i=0; i<no_of_recv; i++)
	{
		if(recv_list[i] == rid)
		{
			// Receiver found already...
			found=1;
			return found;
		}
	}
	if(no_of_recv < MAX_RECV && regnum == 1)
	{
		recv_list[no_of_recv] = rid;
		no_of_recv++;
	}
	else if(no_of_recv>= MAX_RECV)
	{
		printf("\n total receiver registered...=%d", no_of_recv);
		printf("\n receiver limit reached...");
	}
	return found;
}


int mail_insert(char data[], int usrid, int destrec[], int no_of_recv)
{
	int i=0;
	//printf("\n inserting Mail...");

	if(mail_cnt < 16)
	{
		mailbox[mail_cnt].msgList.sender_proc_id=usrid;
		mailbox[mail_cnt].msgList.num_of_read_completed=no_of_recv;
		mailbox[mail_cnt].msgList.deletedFlag=0;

		strncpy(mailbox[mail_cnt].msgList.mesg, data, strlen(data));

		for(i=0; i<no_of_recv; i++)
		{
			mailbox[mail_cnt].msgList.reclist[i]=destrec[i];
		}

		mail_cnt++;
		printf("\n mailboxq.c:- Mail_cnt=%d", mail_cnt);
	}
	else
	{
		printf("\n Rejecting request...Mail_cnt reached=%d", mail_cnt);
		return MAILBOX_OVERFLOW; // return 8 on overflow...
	}
	return 1;
}


int mail_receive(char *data[], int recid)
{
	int found=0, i,j, k=0, deletion=0;
	int read_done = -1;
	printf("\n Mailboxq.c File: Getting Mail...%d", recid);

	if(mail_cnt == 0)
	{
		printf("\n Mailbox Underflow !!!");
		return -1;
	}
	for(i=0; i<mail_cnt;i++)
	{
		printf("\n i=%d & mailbox[i]=%s", i , mailbox[i].msgList.mesg);
		for(j=0;j<5;j++)
		{
			if(recid == mailbox[i].msgList.reclist[j] &&
					mailbox[i].msgList.deletedFlag == 0 )
			{
				printf("\n Ohh I found the mail for me...");

				mailbox[i].msgList.num_of_read_completed--;
				data[k]=(char *)malloc(sizeof(char ) * 20);
				strncpy(data[k], mailbox[i].msgList.mesg,
						sizeof(mailbox[i].msgList.mesg));
				strcat(data[k], "\0");
				//printf("\n Inserted data k=%d, & msg=%s", k, data[k]);
				k++;
				found=k;

				// to mark read at most once only...
				mailbox[i].msgList.reclist[j]=read_done;

				if(mailbox[i].msgList.num_of_read_completed == 0)
				{
					printf("\n Receiver:-Message read by ALL...deleting...");
					mailbox[i].msgList.deletedFlag=1;
					deletion =1;
					// call garbage collection...
					make_mailbox(i);
					i--;
					j=0;
				}
			}
		}
	}
	if(found==0)
		printf("\n No Email for you..Sorry");

	return found;
}


void make_mailbox(int pos)
{
	printf("\n Clearing Garbage from Mailbox...");
	int i, k, j;
	if(pos != -1)
	{
		for(i=pos, j=i+1; j<=mail_cnt; i++, j++)
		{
			strcpy(mailbox[i].msgList.mesg, mailbox[j].msgList.mesg);
			//Sender ID copy...
			mailbox[i].msgList.sender_proc_id=mailbox[j].msgList.sender_proc_id;
			//Receiver ID Copy...
			mailbox[i].msgList.rec_proc_id=mailbox[j].msgList.rec_proc_id;
			//Read Complete & DeleteFlags...
			mailbox[i].msgList.num_of_read_completed=mailbox[j].msgList.
					num_of_read_completed;
			mailbox[i].msgList.deletedFlag=mailbox[j].msgList.deletedFlag;
			// Receiver's List
			for(k=0; k<5; k++)
			{
				mailbox[i].msgList.reclist[k]=mailbox[j].msgList.reclist[k];
			}
			//mail_cnt--;
		}

		mail_cnt--;
		printf("\n After garbage: Mail-cnt=%d", mail_cnt);
		for(i=0; i<mail_cnt; i++)
			printf("\n Mailbox shifted:i=%d & MSG=%s", i,mailbox[i].msgList.mesg);
	}
	else
		printf("\n No Message in Mailbox for that Position=%d...", pos);
}








