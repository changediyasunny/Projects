#include<stdio.h>
#include "mailboxq.h"
#include "pm.h"
#include <minix/callnr.h>
#include <minix/com.h>
#include <minix/type.h>
#include <minix/endpoint.h>
#include <minix/type.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "param.h"
#include<lib.h>


/*-----MY System Calls Start Here.....*/

int do_minit()
{
	printf("\n SYSCALL: MINIT...mailbox.c file");
	mail_init();
	return 1;
}

int do_msearch(void)
{
	//SYSCALL: 58
	int sid, regnum, ret;

	printf("\n SYSCALL: MSEARCH...");

	sid=m_in.m7_i1;
	regnum=m_in.m7_i2;

	ret=add_mail_list(sid, regnum);

	return ret;
}


int do_msender(void)
{
	//SYSCALL: 108
	int sid, regnum;
	int ret;

	printf("SYSCALL: MSENDER: into Mailbox.c");

	sid = m_in.m7_i1;
	regnum = m_in.m7_i2;
	ret=add_sender_list(sid, regnum);

	return ret;

}

int do_mreciver(void)
{
	//SYSCALL: 109
	int rid, regnum;
	int ret;

	printf("SYSCALL: Mreciver: into Mailbox.c");

	rid = m_in.m7_i2;
	regnum= m_in.m7_i1;

	ret=add_recv_list(rid, regnum);

	return ret;

}

int do_usrsend(void)
{
	//actual syscall to deposite message; 69:USRSEND
	int ret;
	int  SID, destrec[5], no_of_recv;
	printf("\n SYSCALL: USRSEND system call in mailbox...");

	char mesg[100];

	//copy the message from PM-->MESG
	sys_datacopy(m_in.m_source, (vir_bytes) m_in.m7_p1, PM_PROC_NR,
			(vir_bytes)mesg, 100);

	strcat(mesg,"\0");

	//copy sendID & no. of receivers...
	SID=m_in.m7_i1;
	no_of_recv=m_in.m7_i2;

	destrec[0]=m_in.m7_i3;
	destrec[1]=m_in.m7_i4;
	destrec[2]=m_in.m7_i5;

	printf("\n mailbox.c: MSG=%s & SID=%d & NO-REC=%d", mesg,SID,no_of_recv);

	if(strlen(mesg) > MAX_MESG_LEN)
			return MSG_LEN_OVERFLOW_ERROR;

	ret=mail_insert(mesg, SID, destrec, no_of_recv);  //called to mailboxq.c

	return ret;
}


int do_usrrecive()
{
	printf("\n SYSCALL: USRRECIVE system call in mailbox...");
	int i;
	char *mesg1[100], temp[100];
	int rd, ret;

	bzero(temp,100);

	rd=m_in.m7_i2;

	ret=mail_receive(mesg1, rd);

    if(ret > 0)
	{
		//printf("\n message received in PM=%s ", mesg1[0]);
		i=0;
    	while(i < ret)
    	{
    		strcat(temp, mesg1[i]);
    		strcat(temp, "->");
    		i++;
    	}

		sys_datacopy(PM_PROC_NR, (vir_bytes) temp,
		    		m_in.m_source,(vir_bytes) m_in.m7_p1, MAX_MESG_LEN);
		return ret;
	}
	else
		printf("\n mail nahi hai bhaiii....");

	return 0;
}

int do_snap()
{
	int i;
	//SYSCALL: 35


	char temp1[20], temp2[20];
	bzero(temp2,20);
	bzero(temp1,20);

	for(i=0; i<total_user; i++)
	{
		sprintf(temp1,"%d", user_list[i]);
		strcat(temp2, temp1);
		strcat(temp2, "->");
	}
	strcat(temp2,"\0");
	//M7_P2....used as sender list store..

	sys_datacopy(PM_PROC_NR, (vir_bytes) temp2,
				    		m_in.m_source,(vir_bytes) m_in.m7_p2, strlen(temp2));

	char temp[500];
	bzero(temp, 500);

	for(i=0; i<mail_cnt; i++)
	{
		//Message copy...
		strcat(temp, mailbox[i].msgList.mesg);
		strcat(temp, "->");
	}
	//M7_P1...used as message
	strcat(temp, "\0");
	sys_datacopy(PM_PROC_NR, (vir_bytes) temp,
					    		m_in.m_source,(vir_bytes) m_in.m7_p1,strlen(temp));


	return 0;
}

int do_mgarbage()
{
	/*
	 * 56 : MGARBAGE
	 * */
	int val = m_in.m7_i1;

	printf("\nSYSCALL: Mailbox Garbage Program :)...");
	if(val == 1)
	{
		printf("\n Garbage collecting Sender Lists...");
		memset( sender_list,0, sizeof(sender_list) );
		no_of_sender=0;
		return val;
	}
	else if(val == 2)
	{
		printf("\n Garbage collecting Receiver Lists...");
		memset( recv_list,0, sizeof(recv_list) );
		no_of_recv = 0;
		return val;
	}
	else if(val == -1)
	{
		printf("\n Garbage collecting MAILBOX Lists...");
		memset( mailbox, 0, sizeof(mailbox));
		mail_cnt=0;
		return val;
	}

	return val;
}

int do_halt_recover()
{
	/*
	 * 35: Mailbox do_halt_recover
	 *
	 * */
	int i, j, k, count;
	char temp1[100], temp2[100];
	bzero(temp1, 100);
	bzero(temp2, 100);

	for(j=0; j<total_user; j++)
	{
		count=0;
		for(i=0; i<mail_cnt; i++)
		{
			for(k=0; k<3; k++)
			{
				if(user_list[j] == mailbox[i].msgList.reclist[k])
				{
					count++;
					snapshot[j].recv_id=user_list[j];
					snapshot[j].pending_mesg=count;
				}
			}
		}
	}
	for(i=0; i<j; i++)
	{
		sprintf(temp1,"%d", snapshot[i].recv_id);
		strcat(temp2,temp1);
		strcat(temp2,"/");

		sprintf(temp1, "%d", snapshot[i].pending_mesg);
		strcat(temp2,temp1);
		strcat(temp2, "->");
	}

	sys_datacopy(PM_PROC_NR, (vir_bytes) temp2,
					    		m_in.m_source,(vir_bytes) m_in.m7_p1, strlen(temp2));

	return 0;
}


