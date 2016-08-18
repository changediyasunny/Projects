#include<stdio.h>
#include "mail_ipc.h"
#include "pm.h"
#include <minix/callnr.h>
#include <minix/com.h>
#include <minix/type.h>
#include <stdlib.h>
#include <string.h>
#include "param.h"
#include<lib.h>


/*-----MY System Calls Start Here.....*/


int do_minit()
{
	printf("\n SYSCALL: MINIT...mailbox.c file");
	mailbox_init();
	return 1;
}

int do_msearch()
{
	//103: MSEARCH
	int user_id, regnum, mailbox_id, ret=0, flag;
	char name[100];
	bzero(name,100);

	printf("\n SYSCALL: MSEARCH Function...");

	user_id=m_in.m7_i1;
	regnum=m_in.m7_i2;
	mailbox_id=m_in.m7_i3;
	flag=m_in.m7_i4;

	sys_datacopy(m_in.m_source, (vir_bytes) m_in.m7_p1, PM_PROC_NR,
													(vir_bytes)name, 100);

	printf("\n Mail_ipc.c: MSEARCH: name=%s, sid=%d & regnum=%d & MID=%d & flag=%d",
			name,user_id, regnum, mailbox_id,flag);

	ret=search_list(name, user_id, regnum, mailbox_id, flag);
	printf("\n MSEARCH: ret=%d", ret);
	return ret;
}

int do_remuser()
{
	int user_id, mailbox_id, user_flag;

	user_id=m_in.m7_i1;
	mailbox_id=m_in.m7_i2;
	user_flag=m_in.m7_i3;

	int ret;
	ret = remove_user(user_id, mailbox_id, user_flag);
	return ret;
}


int do_mcreate()
{
	//Create New Mailbox...
	printf("\n SYSCALL: MINIT...mail_ipc.c file");
	int ret =1;

	int ownID, no_of_sender, no_of_recv, type;
	char name[100];

	ownID = m_in.m7_i1;
	no_of_sender = m_in.m7_i2;
	no_of_recv = m_in.m7_i3;
	type = m_in.m7_i4;

	sys_datacopy(m_in.m_source, (vir_bytes) m_in.m7_p1, PM_PROC_NR,
				(vir_bytes)name, 100);

	printf("\nDO-CREATE: send=%d, recv=%d, type=%d, name=%s", no_of_sender, no_of_recv,
			type,name);


	ret = mail_init(ownID, name, no_of_sender, no_of_recv, type);

	if (ret == -1)
	{
		printf("\n Mailbox Count reached it's peak...");
		return ret;
	}
	printf("\n Mail_IPC: ret received...%d", ret);

	return ret;
}

/*
 * Change permissions of Owner/ Sender/ Receiver
 * */
int do_changep(void)
{
	//SYSCALL : 97
	printf("\n SYSCALL; DO_CHANGEP");
	char perm[100];
	int ID, regnum, mailbox_id;
	int ret;

	ID = m_in.m7_i1;
	mailbox_id = m_in.m7_i2;
	regnum = m_in.m7_i3;

	sys_datacopy(m_in.m_source, (vir_bytes) m_in.m7_p1, PM_PROC_NR,
				(vir_bytes)perm, 100);

	ret = change_perm(ID, mailbox_id, perm);

	return ret;
}

int do_snap()
{
	int i, mailbox_id;

	char temp1[100], temp2[100];
	bzero(temp2,100);
	bzero(temp1,100);

	mailbox_id=m_in.m7_i1;

	if(mailbox_id == OWNER_LIST)  //Owner List Display option...
	{
		printf("\n Owner List Display...");

		for(i=0; i<no_of_owner; i++)
		{
			sprintf(temp1,"%d", owner[i].OID);
			strcat(temp2, temp1);
			strcat(temp2, "(");
			strcat(temp2,owner[i].priv);
			strcat(temp2, ")");
			strcat(temp2, "-->");
		}
		strcat(temp2,"\0");
		printf("\n Owner-List=%s", temp2);
		sys_datacopy(PM_PROC_NR, (vir_bytes) temp2,
					    		m_in.m_source,(vir_bytes) m_in.m7_p2, sizeof(temp2));
	}
	else //Any Other Sender/Receiver List display...
	{
		printf("\n Send/Recv Display...");

		for(i=0; i < mailbox[mailbox_id]->total_user; i++)
		{
			sprintf(temp1,"%d", mailbox[mailbox_id]->user_list[i].user_id);
			strcat(temp2, temp1);
			sprintf(temp1, "%d", mailbox[mailbox_id]->user_list[i].send_bit);
			strcat(temp2, "(");
			strcat(temp2, temp1);
			strcat(temp2, "/");
			sprintf(temp1, "%d", mailbox[mailbox_id]->user_list[i].recv_bit);
			strcat(temp2, temp1);
			strcat(temp2, ")");
			strcat(temp2, "->");
		}
		strcat(temp2,"\0");
		//M7_P2....used as sender list store..
		printf("\n UserLIST in MAIL-IPC: %s", temp2);
		sys_datacopy(PM_PROC_NR, (vir_bytes) temp2,
					    		m_in.m_source,(vir_bytes) m_in.m7_p2, strlen(temp2));


		char temp[500];
		bzero(temp, 500);
		printf("\n MAIL-IPC: Display of Actual Message;");
		for(i=0; i<mailbox[mailbox_id]->mail_cnt; i++)
		{
			//Message copy...
			strcat(temp, mailbox[mailbox_id]->msgList[i].mesg);
			strcat(temp, "->");
		}
		//M7_P1...used as message
		strcat(temp, "\0");
		sys_datacopy(PM_PROC_NR, (vir_bytes) temp,
					    		m_in.m_source,(vir_bytes) m_in.m7_p1,strlen(temp));
	}

	return 0;
}


int do_msender(void)
{
	//SYSCALL: 108
	int sid, regnum, mailbox_id;
	int ret;

	printf("SYSCALL: MSENDER: into Mail_ipc.c");

	sid = m_in.m7_i1;
	regnum = m_in.m7_i2;
	mailbox_id = m_in.m7_i3;

	printf("\n Mail_ipc.c: MSENDER: sid=%d & regnum=%d & MID=%d", sid, regnum, mailbox_id);

	if(mailbox_id == -2)  //-2 for Owner Registration...
	{
		// It's a owner registration...goto Owner List
		ret = add_owner_list(sid, regnum, mailbox_id);
	}
	else
	{
		//It's a sender registration...goto Sender List
		ret = add_user_list(sid, regnum, mailbox_id);
	}

	return ret;

}

int do_mreciver(void)
{
	//SYSCALL: 109
	int rid, regnum, mailbox_id;
	int ret;

	printf("SYSCALL: Mreciver: into mail_ipc.c");

	rid = m_in.m7_i2;
	regnum= m_in.m7_i1;
	mailbox_id=m_in.m7_i3;

	printf("\n mail_ipc.c: MRECEIVER: sid=%d & regnum=%d & MbID=%d", rid, regnum, mailbox_id);
	ret=add_recv_list(rid, regnum, mailbox_id);

	return ret;
}


int do_usrsend(void)
{
	//actual syscall to deposite message; 69:USRSEND
	int ret;
	int destrec[5], mailbox_id, no_of_recv;
	printf("\n SYSCALL: USRSEND system call in mailbox...");

	char mesg[100];

	//copy the message from PM-->MESG
	sys_datacopy(m_in.m_source, (vir_bytes) m_in.m7_p1, PM_PROC_NR,
			(vir_bytes)mesg, 100);

	strcat(mesg,"\0");

	//copy recv count & ID of it's mailbox...
	mailbox_id=m_in.m7_i1;
	no_of_recv=m_in.m7_i2;

	destrec[0]=m_in.m7_i3;
	destrec[1]=m_in.m7_i4;
	destrec[2]=m_in.m7_i5;

	printf("\n mail_ipc.c: MSG=%s & MID=%d & NO-REC=%d", mesg,mailbox_id,no_of_recv);

	ret=mail_insert(mesg, no_of_recv, destrec, mailbox_id);  //called to mailboxq.c

	return ret;
}

int do_usrrecive()
{
	printf("\n SYSCALL: USRRECIVE system call in mailbox...");
	int i, mailbox_id;
	char *mesg1[100], temp[100];
	int rd, ret;

	bzero(temp,100);

	mailbox_id=m_in.m7_i1;
	rd=m_in.m7_i2;

	ret=mail_receive(mesg1, rd, mailbox_id);

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
		printf("\n mail nahi hai bhaiii...");

	return 0;
}

int do_denyaccess()
{
	//110: DENYACCESS
	int userid, mailbox_id, bit, flag;
	int ret;

	userid=m_in.m7_i1;
	mailbox_id=m_in.m7_i2;
	bit=m_in.m7_i3;
	flag=m_in.m7_i4;

	ret=deny_access(userid,mailbox_id,bit,flag);

	return ret;
}

int do_mgarbage()
{
	/*
	 * 56 : MGARBAGE
	 * */
	int val = m_in.m7_i1;
	int mailbox_id=m_in.m7_i2;

	printf("\nSYSCALL: Mailbox Garbage Program :)...");
	if(val == 1)  //CLEAR OWNER LIST
	{
		printf("\n Garbage collecting Owner Lists...");
		memset( owner_list,0, sizeof(owner_list) );
		no_of_owner=0;
		return val;
	}
	else if(val == 2)  //CLEAR USER LIST
	{
		printf("\n Garbage collecting Receiver Lists...");
		memset(mlist, 0, sizeof(struct mailbox_name_id_list) );
		//memset(user_list,0, sizeof(user_list) );
		//total_public_user = 0;
		return val;
	}
	else if(val == -1 && mailbox_id >= 0)  //CLEAR MAILBOX
	{
		printf("\n Garbage collecting MAILBOX Lists...");
		free(mailbox[mailbox_id]);
		/*free(mailbox[mailbox_id]->);
		mailbox[mailbox_id]->mail_cnt=0;
		mailbox[mailbox_id]->msgList[count].mesg
		mailbox[mailbox_id]->msgList[count].reclist[i]
		mailbox[mailbox_id]->msgList[count].mesg*/
		return val;
	}

	return val;
}

/*int do_halt_recover()
{
	//35: Mailbox do_halt_recover

	int i, j, k, count;
	char temp1[100], temp2[100];
	bzero(temp1, 100);
	bzero(temp2, 100);

	for(j=0; j<no_of_recv; j++)
	{
		count=0;
		for(i=0; i<mail_cnt; i++)
		{
			for(k=0; k<3; k++)
			{
				if(recv_list[j] == mailbox[i].msgList.reclist[k])
				{
					count++;
					snapshot[j].recv_id=recv_list[j];
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
*/

