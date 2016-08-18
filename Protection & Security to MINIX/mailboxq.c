#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "mail_ipc.h"
#include<minix/com.h>
#include <minix/type.h>
#include <unistd.h>
#include <minix/callnr.h>
#include<minix/ipc.h>

void mailbox_init()
{
	printf("\n SYSCALL: mailbox_init function...");

	memset(owner_list, 0, sizeof(owner_list) );
	memset(mlist, 0, sizeof(struct mailbox_name_id_list) );

	printf("\n size allocated to Mlist, Owner list & public list....");
	mailbx_num = 0;
	no_of_owner = 0;
}


int mail_init(int ownID, char name[], int no_of_sender, int no_of_recv, int type)
{
	printf("\n SYSCALL: MAIL-INIT Function...");

	int status=0;

	if(mailbx_num < MAX_MAILBOX)
	{

		if((mailbox[mailbx_num]=(struct mailbox_node *)malloc(sizeof(struct mailbox_node)))
				== NULL)
		{
			printf("\nMAIL-INIT: Memory allocation failed...");
			return -1;
		}

		mailbox[mailbx_num]->total_user= 0;

		/*
		 * General Mailbox assignment...
		 * */
		mailbox[mailbx_num]->mail_cnt=0;
		mailbox[mailbx_num]->mailbox_type=type;
		strncpy(mailbox[mailbx_num]->name, name, strlen(name));

		/*
		 * MLIST data assignment...
		 * NAME, MID, OWNER-ID, TYPE
		 * */
		strncpy(mlist[mailbx_num].mname, name, strlen(name));
		mlist[mailbx_num].MID = mailbx_num;
		mlist[mailbx_num].ownid=ownID;
		mlist[mailbx_num].type=type;


		printf("\n MAIL-INIT: send=%d, recv=%d, type=%d, name=%s", mailbox[mailbx_num]->no_of_sender,
				mailbox[mailbx_num]->no_of_recv,mailbox[mailbx_num]->mailbox_type,
				mailbox[mailbx_num]->name);

		mailbx_num++;
	}
	else
	{
		// mailbox count reached max...
		status = -1;
		return status;
	}
	return status;
}

int remove_user(int user_id, int mailbox_id, int user_flag)
{
	int found=0, pos, i;

	if(user_flag == 1)
	{
		// It's a normal user removal based on mailbox_id...
		for(i=0; i < mailbox[mailbox_id]->total_user; i++)
		{
			if(user_id == mailbox[mailbox_id]->user_list[i].user_id)
			{
				printf("Found the user ID, now delete it...");
				pos=i;
				found = 1;
				delete_user(pos, mailbox_id);
			}
		}
	}
	else if(user_flag == 3 && mailbox_id == -2)
	{
		//it's a owner removal based on MLIST & Owner List Both...
		for(i=0; i<no_of_owner; i++)
		{
			if(user_id == owner[i].OID)
			{
				printf("\n Found the owner to remove...%d", owner[i].OID);
				pos = i;
				delete_user(pos,mailbox_id);
			}
		}

	}

	return found;

}

void delete_user(int pos, int mailbox_id)
{
	int i=0, j;
	printf("\n Deleting the User Now !!!");

	if(mailbox_id == -2)   //OWNER List deletion...
	{
		printf("\n Owner delete from Owner_list...");
		for(i=pos, j=i+1; j <= no_of_owner; i++, j++)
		{
			owner[i].OID=owner[j].OID;
			strcpy(owner[i].priv, owner[j].priv);
		}
		no_of_owner--;
	}
	else // Normal Sender/Receiver Deletion...
	{
		printf("\n Normal User delete ...");
		for(i=pos, j=i+1; j <= mailbox[mailbox_id]->total_user; i++, j++)
		{
			mailbox[mailbox_id]->user_list[i].user_id=mailbox[mailbox_id]->user_list[j].user_id;
			mailbox[mailbox_id]->user_list[i].send_bit=mailbox[mailbox_id]->user_list[j].send_bit;
			mailbox[mailbox_id]->user_list[i].recv_bit=mailbox[mailbox_id]->user_list[j].recv_bit;
			mailbox[mailbox_id]->user_list[i].access=mailbox[mailbox_id]->user_list[j].access;
		}
		mailbox[mailbox_id]->total_user--;
	}
}

int search_list(char name[10], int id, int regnum, int mailbox_id, int flag)
{
	int ret=NOT_FOUND, j=0, i, retID=-1, mark=-2;
	char temp[5];
	bzero(temp,5);

	int search = regnum;

	printf("\n Into Search-LISt Function...");

	//For MLIST Search....
	if(mailbox_id == MLIST)  //MLILST=-3
	{
		printf("\n MLIST SEARCH...");

		if(id >=0)
		{
			if(flag == -2)
			{
				printf("\n SEARCH based only on OwnerID...");
				for(i=0; i<mailbx_num; i++)
				{
					if ( id == mlist[i].ownid )
					{
						mark=mlist[i].MID;  //assign unique mailbox id...
						break;
					}
				}
				return mark;
			}
			else
			{
				printf("\n MLIST-Search:Based on NAME + OwnerID:");
				for(i=0; i<mailbx_num; i++)
				{
					if ( id == mlist[i].ownid && (strcmp(name,mlist[i].mname) == 0) )
					{
						mark=1;
						retID=mlist[i].MID;  //assign unique mailbox id...
						break;
					}
				}
				return retID;

			}
		}
		else if(id <0)  //-1 sent from user space...
		{
			printf("\nMLIST Search: Based on ONLY Name...");
			for(i=0; i<mailbx_num; i++)
			{
				if ( strcmp(name,mlist[i].mname) == 0)
				{
					if(flag == -1)  //OID
					{
						//return the Owner ID from MLIST...
						printf("\n Will return OID...");
						retID=mlist[i].ownid;
					}
					else if(flag == 0)  //MID
					{
						//Return MID from MLIST...
						printf("\n Will return MID...");
						retID=mlist[i].MID;
					}
					else if(flag == 1)  //TYP
					{
						//Return TYP from MLIST
						if(mlist[i].type == PUBLIC)
						{
							printf("\n Public mailbox Found...Will return type...");
							retID=mlist[i].MID;
						}
					}
				}
			}
		}
		return retID;
	}
	else if(mailbox_id == OWNER_DATA) //OWNER_DATA=-2
	{
		//Owner list search with 1.either Owner & 2.Permissions
		if(search >= CM_FOUND && search <= RM_FOUND)
		{
			//searching for permissions...
			printf("\nMailboxq.c: Special case PERM checking:");
			for(i=0; i < no_of_owner; i++)
			{
				if(owner[i].OID == id)
				{
					if(search == CM_FOUND)  // check for Searching...
					{
						strcpy(temp, owner[i].priv);
						strcat(temp, "\0");
						printf("\n MAilboxq.c: Checking for CREATE Access...%s", temp);
						while(temp[j] != '\0')
						{
							if(temp[j] == 'c')
								ret = FOUND;
							j++;
						}
					}
					else if(search == RW_FOUND)
					{
						strcpy(temp, owner[i].priv);
						strcat(temp, "\0");

						while(temp[j] != '\0')
						{
							if(temp[j] == 's')
								ret = FOUND;
							j++;
						}
					}
					else if(search == RM_FOUND)
					{
						strcpy(temp, owner[i].priv);
						strcat(temp, "\0");

						while(temp[j] != '\0')
						{
							if(temp[j] == 'r')
								ret = FOUND;
							j++;
						}
					}
				}
			}

			return ret;
		}
		else  //searching for Owner ID
		{
			printf("\nMailbox1.c: Owner Searching Starts here...");
			for(i=0; i < no_of_owner; i++)
			{
				if(owner[i].OID == id)
				{
					// if Owner id found...
					printf("\n Owner found...");
					ret = FOUND;
				}
			}
		}
	}
	else //searching for sender & receiver...
	{

		for(i=0; i< mailbox[mailbox_id]->total_user; i++)
		{
			if(id == mailbox[mailbox_id]->user_list[i].user_id)
			{
				printf("\n User Found...");
				ret=mailbox_id;
				break;
			}
		}
	}
	printf("\nMailboxq.c: ret = %d", ret);
	return ret;
}

int deny_access(int userid, int mailbox_id, int bit, int flag)
{
	int i, found=OFF, mark=-1;

	if(flag == MAKING)
	{
		printf("\n DENY ACCESS: updating access....");
		for(i=0; i<mailbox[mailbox_id]->total_user; i++)
		{
			if(userid == mailbox[mailbox_id]->user_list[i].user_id)
			{
				if(bit == SEND_BIT)
				{
					printf("\n Removing SEND Access for User=%d", userid);
					mailbox[mailbox_id]->user_list[i].send_bit=OFF;

				}
				else if(bit == RECV_BIT)
				{
					printf("\n Removing RECV Access for User=%d", userid);
					mailbox[mailbox_id]->user_list[i].recv_bit=OFF;
				}

			}
		}
		return 1;
	}
	else if(flag == CHKING)
	{
		printf("\n DENY ACCESS: NORMAL list checking....");
		for(i=0; i<mailbox[mailbox_id]->total_user; i++)
		{
			if(userid == mailbox[mailbox_id]->user_list[i].user_id)
			{
				if(bit == SEND_BIT && mailbox[mailbox_id]->user_list[i].send_bit == ON)
				{
					printf("\n validating SEND Access for User=%d", userid);
					return ON;

				}
				else if(bit == RECV_BIT && mailbox[mailbox_id]->user_list[i].recv_bit == ON)
				{
					printf("\n Validating RECV Access for User=%d", userid);
					return ON;
				}

			}
		}
	}
	else if(flag == UPDT)
	{
		//UPDATE SEND & RECV ACCESS of normal user....
		printf("\n Allow ACCESS: updating access....");
		for(i=0; i<mailbox[mailbox_id]->total_user; i++)
		{
			if(userid == mailbox[mailbox_id]->user_list[i].user_id)
			{
				if(bit == SEND_BIT)
				{
					printf("\n Updating SEND Access for User=%d", userid);
					mailbox[mailbox_id]->user_list[i].send_bit=SEND_BIT;

				}
				else if(bit == RECV_BIT)
				{
					printf("\n Updating RECV Access for User=%d", userid);
					mailbox[mailbox_id]->user_list[i].recv_bit=RECV_BIT;
				}
			}
		}
		return 1;
	}
	else if(flag == PUBCHK)
	{
		printf("\n DENY ACCESS: PUBLIC list checking....");
		for(i=0; i<mailbox[mailbox_id]->total_user; i++)
		{
			if(userid == mailbox[mailbox_id]->user_list[i].user_id)
			{
				mark=0;
				if(bit == SEND_BIT && mailbox[mailbox_id]->user_list[i].send_bit == ON)
				{
					printf("\n validating SEND Access for User=%d", userid);
					return ON;
				}
				else if(bit == RECV_BIT && mailbox[mailbox_id]->user_list[i].recv_bit == ON)
				{
					printf("\n Validating RECV Access for User=%d", userid);
					return ON;
				}
			}
		}
		if(mark == -1) //Public List Insertion...
		{
			printf("\n MARK=0 & Insert to public list...");
			mailbox[mailbox_id]->user_list[i].user_id=userid;
			mailbox[mailbox_id]->user_list[i].send_bit=ON;
			mailbox[mailbox_id]->user_list[i].recv_bit=ON;
			mailbox[mailbox_id]->total_user++;
			return mark;
		}
	}
	return found;
}

int add_owner_list(int ownerID, int regnum, int mailbox_id)
{
	int found = 0, i;

	char temp[5];
	bzero(temp,5);

	printf("\n mailboxq.c file: into add_owner list");

	for(i=0; i < no_of_owner; i++)
	{
		printf("\n mailboxq.c file: searching for Owner ID...");
		if(owner[i].OID == ownerID)
		{
			// if Duplicate sender id found...then
			printf("\n Duplicate Owner ID found...");
			found=1;
			if(regnum == 0)  // check for Searching...
			{
				printf("\n Case of searching & Found sender ID:");
				return 0;  // trying to search & found...
			}
		}
	}

	if(regnum == REGISTER && found == 0 && no_of_owner < 10)
	{
		// Then register the sender...

		owner[no_of_owner].OID = ownerID;
		/*
		 * Priv: C-create, S-sende/receive, R-remove-msg
		 * */
		strcpy(owner[no_of_owner].priv, "csr");

		printf("\n Owner Added...%d", owner[no_of_owner].OID );
		no_of_owner++;

	}
	else if( no_of_owner >= 10)
	{
		printf("\n Owner limit reached...");
		printf("\n total Owner registered...=%d", no_of_owner);
	}
	else if(found == 1 && regnum == REGISTER)
	{
		// Case of Duplication...
		return 1;
	}
	else if(regnum == SEARCH && found == 0)
	{
		// Case of search fail..Sender does not exists..
		return -1;
	}

	return found;
}

int change_perm(int ID, int mailbox_id, char *perm)
{
	int k, found=0;
	printf("\n Mailboxq.c: Change_perm function...");
	if(mailbox_id == -2)  //It's a owner perm change
	{
		for(k=0; k < no_of_owner; k++)
		{
			if(ID == owner[k].OID)
			{
				//Owner Found...
				found=1;
				bzero(owner[k].priv, 5);
				strcpy(owner[k].priv,perm);
				printf("\nMailboxq.c: Owner[k]->priv=%s", owner[k].priv);
			}
		}
		if(found == 0)
		{
			return -1;
		}
	}
	return found;
}


int add_user_list(int sid, int regnum, int mailbox_id)
{
	int found=0, i;
	printf("\n mailboxq.c file: into add_user list");

	if(mailbox_id < 0)
	{
		printf("\n Mailbox Index is Invalid & Negative...");
		return -1;
	}

	for(i=0; i < mailbox[mailbox_id]->total_user; i++)
	{
		printf("\n mailboxq.c file: searching for User ID...");
		if(mailbox[mailbox_id]->user_list[i].user_id == sid)
		{
			// if Duplicate sender id found...then
			printf("\n Duplicate User ID found...");
			found=1;
			if(regnum == 0)  // check for Searching...
			{
				printf("\n Case of searching & Found sender ID:");
				return 0;  // trying to search & found...
			}
		}
	}
	/*
	 * regnum =1 indicates new register for sender
	 * Found = 0 indicates sender not available
	 * */
	if(regnum == 1 && found == 0 && mailbox[mailbox_id]->total_user < MAX_USER)
	{
		// Then register the sender...
		printf("\n Adding User....");
		mailbox[mailbox_id]->user_list[mailbox[mailbox_id]->total_user].user_id=sid;
		mailbox[mailbox_id]->user_list[mailbox[mailbox_id]->total_user].send_bit=ON;
		mailbox[mailbox_id]->user_list[mailbox[mailbox_id]->total_user].recv_bit=ON;
		mailbox[mailbox_id]->user_list[mailbox[mailbox_id]->total_user].access=ALLOW;

		printf("\n User Added...%d", mailbox[mailbox_id]->
								user_list[mailbox[mailbox_id]->total_user].user_id );

		mailbox[mailbox_id]->total_user++;

	}
	else if( mailbox[mailbox_id]->total_user >= MAX_USER)
	{
		printf("\n User limit reached...");
		printf("\n total User registered...=%d", mailbox[mailbox_id]->total_user);
	}
	else if(found == 1 && regnum == 1)
	{
		// Case of Duplication...
		printf("\nCase of duplication User Id...");
		return 1;
	}
	else if(regnum == 0 && found == 0)
	{
		// Case of search fail..Sender does not exists..
		printf("\n case of Search fail & User not there...");
		return -1;
	}

	return found;
}

int add_recv_list(int rid, int regnum, int mailbox_id)
{
	int found=0, i;
	printf("\n mailboxq.c file: into add_recv list");

	for(i=0; i < mailbox[mailbox_id]->no_of_recv; i++)
	{
		if(mailbox[mailbox_id]->recv_list[i] == rid)
		{
			// if Duplicate receiver id found...then
			found=1;
			if(regnum == SEARCH)  // check for Searching...
			{
				printf("\nRECV: Case of searching & Found receiver ID:");
				return 0;  // trying to search & found...
			}
		}
	}
	/*
	 * regnum =1 indicates new register for sender
	 * Found = 0 indicates sender not available
	 * */
	if(regnum == REGISTER && found == 0 && mailbox[mailbox_id]->no_of_recv < MAX_RECV)
	{
		// Then register the sender...

		mailbox[mailbox_id]->recv_list[mailbox[mailbox_id]->no_of_recv] = rid;
		printf("\n Sender Added...%d", mailbox[mailbox_id]->
								recv_list[mailbox[mailbox_id]->no_of_recv] );
		mailbox[mailbox_id]->no_of_recv++;

	}
	else if( mailbox[mailbox_id]->no_of_recv >= MAX_RECV)
	{
		printf("\n sender limit reached...");
		printf("\n total sender registered...=%d", mailbox[mailbox_id]->no_of_recv);
	}
	else if(found == 1 && regnum == REGISTER)
	{
		// Case of Duplication...
		printf("\nCase of duplication Receiver Id...");
		return 1;
	}
	else if(regnum == 0 && found == 0)
	{
		// Case of search fail..Sender does not exists..
		printf("\n case of Search fail & receiver not there...");
		return -1;
	}

	return found;
}


int mail_insert(char data[], int no_of_recv, int destrec[], int mailbox_id)
{
	int i, done=0;
	int count=mailbox[mailbox_id]->mail_cnt;

	if(mailbox[mailbox_id]->mail_cnt < 16)
	{
		//mailbox[mailbox_id]->mailbox_type=1;
		//mailbox[mailbox_id]->msgList[mail_cnt].sender_proc_id=usrid;

		mailbox[mailbox_id]->msgList[count].num_of_read_completed=no_of_recv;
		mailbox[mailbox_id]->msgList[count].deletedFlag=0;

		strncpy(mailbox[mailbox_id]->msgList[count].mesg, data, strlen(data));

		for(i=0; i < no_of_recv; i++)
		{
			mailbox[mailbox_id]->msgList[count].reclist[i]=destrec[i];
		}

		printf("\n mailboxq.c:-msg=%s", mailbox[mailbox_id]->msgList[count].mesg);
		mailbox[mailbox_id]->mail_cnt++;
		done=1;
	}
	else
	{
		printf("\n Rejecting...Mail_cnt reached=%d", mailbox[mailbox_id]->mail_cnt);
		return -1; // return 8 on overflow...
	}
	return done;
}


int mail_receive(char *data[], int recid, int mailbox_id)
{
	int found=0, i,j, k=0, deletion=0;
	int read_done = -1;
	printf("\n Mailboxq.c File: Getting Mail...%d", recid);

	if(mailbox[mailbox_id]->mail_cnt == 0)
	{
		printf("\n Mailbox Underflow !!!");
		return -1;
	}

	for(i=0; i < mailbox[mailbox_id]->mail_cnt; i++)
	{
		for(j=0;j<5;j++)
		{
			if(recid == mailbox[mailbox_id]->msgList[i].reclist[j]
							&&
				mailbox[mailbox_id]->msgList[i].deletedFlag == 0 )
			{
				printf("\n Ohh I found the mail for me...");

				mailbox[mailbox_id]->msgList[i].num_of_read_completed--;

				data[k]=(char *)malloc(sizeof(char ) * 20);
				strncpy(data[k], mailbox[mailbox_id]->msgList[i].mesg,
						sizeof(mailbox[mailbox_id]->msgList[i].mesg));
				strcat(data[k], "\0");
				printf("\n Inserted data k=%d, & msg=%s", k, data[k]);
				k++;
				found=k;

				// to mark read at most once only...
				mailbox[mailbox_id]->msgList[i].reclist[j] = read_done;

				if(mailbox[mailbox_id]->msgList[i].num_of_read_completed == 0)
				{
					printf("\n Receiver:-Message read by ALL...deleting...");
					mailbox[mailbox_id]->msgList[i].deletedFlag=1;
					deletion =1;
					// call garbage collection...
					make_mailbox(i, mailbox_id);
					i--;
					j=0;
				}
			}
		}
	}
	if(found==0)
	{
		printf("\n No Email for you..Sorry");
		return -1;
	}


	return 1;
}


void make_mailbox(int pos, int mailbox_id)
{
	printf("\n Clearing Garbage from Mailbox...");
	int i, k, j;
	if(pos != -1)
	{
		for(i=pos, j=i+1; j<=mailbox[mailbox_id]->mail_cnt; i++, j++)
		{
			//Message copy...
			strcpy(mailbox[mailbox_id]->msgList[i].mesg,
					mailbox[mailbox_id]->msgList[j].mesg);
			//Sender ID copy...
			mailbox[mailbox_id]->msgList[i].sender_proc_id=mailbox[mailbox_id]->msgList[j].sender_proc_id;
			//Receiver ID Copy...
			mailbox[mailbox_id]->msgList[i].rec_proc_id=mailbox[mailbox_id]->msgList[j].rec_proc_id;
			//Read Complete & DeleteFlags...
			mailbox[mailbox_id]->msgList[i].num_of_read_completed=mailbox[mailbox_id]->msgList[j].
					num_of_read_completed;
			mailbox[mailbox_id]->msgList[i].deletedFlag=mailbox[mailbox_id]->msgList[j].deletedFlag;
			// Receiver's List
			for(k=0; k<5; k++)
			{
				mailbox[mailbox_id]->msgList[i].reclist[k]=mailbox[mailbox_id]->msgList[j].reclist[k];
			}
			//mail_cnt--;
		}

		mailbox[mailbox_id]->mail_cnt--;
		printf("\n After garbage: Mail-cnt=%d", mailbox[mailbox_id]->mail_cnt);
		for(i=0; i<mailbox[mailbox_id]->mail_cnt; i++)
			printf("\n Mailbox shifted:i=%d & MSG=%s", i,mailbox[mailbox_id]->msgList[i].mesg);
	}
	else
		printf("\n No Message in Mailbox for that Position=%d...", pos);

}








