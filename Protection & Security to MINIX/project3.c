#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include "project2.h"

#define MAX_MSGS 16
#define MAX_RECV 10
#define MAX_SEND 10
#define MAX_MESG_LEN 100


int snapshot(int mailbox_id)
{
	char mails[100], owners[20], users[20], name[10];
	int ret=0;
	bzero(mails, 100);
	strcpy(name,"public1");

	if(mailbox_id == OWNER_LIST)
	{
		//Owner List display...
		ret = sys_snapshot(mails, owners, mailbox_id);
		printf("\n Mailbox Owners Avail:=%s", owners);
	}
	else if(mailbox_id == PUBLIC_LIST)
	{
		int check;
		check=sys_mlist_search_name(name,-1,SEARCH,-3,MID);
		if(check < 0)
		{
			printf("\n Error on Displaying Public mailbox data...");
			return -1;
		}
		ret = sys_snapshot(mails, users, check);
		printf("\n Public Mailbox Messages:=%s", mails);
		printf("\n Public Mailbox Users Avail:=%s", users);
	}
	else
	{

		ret = sys_snapshot(mails, users, mailbox_id);
		printf("\n Mailbox Messages:=%s", mails);
		printf("\n Mailbox Users Avail:=%s", users);
	}

	return ret;
}

int search(int ID, int regnum, int mailbox_id)
{
	int ret;

	ret = sys_msearch(ID, regnum, mailbox_id);

	printf("\n Search Result: %d", ret);
	return ret;
}

int register_user(int mailbox_num)
{
	/*it registers as a sender to PM server*/
	int sid;
	int status;
	int regnum=REGISTER, mailbox_id=0;

	/* regnum = 1........registration
	 * regnum = 0........searching
	 *
	 * */

	printf("\n Enter the User ID:(-1 for no User)");
	scanf("%d", &sid);

	if(sid == -1 || sid == 0)
	{
		printf("\n Don't wanna register sender... ");
		return 0;
	}

	status=sys_msender(sid, REGISTER, mailbox_num);
	if(status == 1)
	{
		printf("\nDuplicate: Sender already there...");
	}
	else if(status == 0)
	{
		printf("User registered status=%d", status);
	}
	return status;
}

void register_receiver(int mailbox_num)
{
	/*it registers as a sender to PM server*/
	int i;
	int status, recid, regnum=REGISTER;

	//printf("\n Manually registering receiver !");
	printf("\n Enter receiver ID:(-1 for no receiver registration)");
	scanf("%d", &recid);

	if(recid == -1 || recid == 0)
	{
		printf("\n Don't wanna register receiver... ");
	}

	//check for receiver present or not...
	status=sys_mreciver(recid, regnum, mailbox_num);

	if(status == 1)
	{
		//already registered....
		printf("\n Duplicate: Receiver already there...");
	}
	else
	{
		printf("\n receiver registered:");
	}
}

void remove_owner()
{
	int i, k, ownID, ret;

	printf("\n Enter Owner ID to remove:");
	scanf("%d", &ownID);
	//owner ID, mailbox_id, OWNER flag
	ret=sys_remove_user(ownID, -2, OWNER);

	printf("\n Owner remove status:%d", ret);
}

void remove_user()
{
	int i, k, ownID, ret;

	printf("\n Enter Owner ID:");
	scanf("%d", &ownID);

	char name[20];
	printf("\n Enter name of Mailbox:");
	scanf("%s", name);

	int chk1;
	// -3 : for Owner MLIST search...
	chk1=sys_mlist_search_name(name,-1, SEARCH, -3, OID);
	//chk1: used to map owner ID of mailbox
	if(chk1 == ownID)
	{
		//Check success & return Owner ID
		int user_id;
		printf("\n Enter Used ID to remove:");
		scanf("%d", &user_id);

		int chk2; //Used as MID to map mailbox_id...
		chk2=sys_mlist_search_name(name, -1,SEARCH, MLIST, MID);

		ret=sys_remove_user(user_id, chk2, USER);
		printf("\n Deleted User stat=%d", ret);
	}
	else
	{
		printf("\n No Such owner against that mailbox name:");
	}
}

void delete_user(int flag)
{
	if(flag == USER)
	{
		remove_user();
	}
	else if(flag == OWNER)
	{
		remove_owner();
	}
}

void deposite_mesg()
{
	printf("\n----Send message to queue.---\n");
    char name[10];

	int sendID;
	printf("\n Enter your Sender ID :");
	scanf("%d",&sendID);

	int mtype;
	printf("\n Type of Mailbox to Access: 1.Public/0. Secure:");
	scanf("%d", &mtype);

    if(mtype == PUBLIC)
    {
    	strcpy(name,"public1");
    	//ACCESS TO PUBLIC MAILBOX...
    	int mailbox_id;
    	mailbox_id=sys_mlist_search_name(name,-1,SEARCH,-3,MID);

    	if(mailbox_id == -1)
    	{
    		printf("\n No Such PUBLIC mailbox exists...");
    	}
    	else if(mailbox_id != -1)
    	{
    		//Public mailbox found...

    		int chk2;
    		chk2=sys_deny_access(sendID, mailbox_id, SEND_BIT, PUBCHK);

    		/* It returns either ON=1 or MARK=-1 for Register...
    		 *
    		 * */

    		if(chk2 == 1 || chk2 == -1)
    		{
    			//SEND Bit is ON...OR User registered...
    			char recvID[50];
	       	    bzero(recvID,50);
    			printf("\n Enter Receiver's ID for message:(Only 3/msg)");
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
    			    status = sys_mdeposite(sendID, recvID, msg, mailbox_id);

    			    if(status == -1)
    			    {
    			    	printf("\n Mailbox Overflow....System Halted");
    			    	printf("\n Goto Admin calls to clear it...");
    			    }
    			    else if(status == 1)
    			    {
    			    	printf("\n Status:Deposite_msg : %d\n", status);
    			    }
    			    else
    			    {
    			    	printf("\n Some error depositing Message ...");
    			    }
    			}
    		}
    		else if(chk2 == 0)
    		{
    			//SEND Bit is not set...OFF
    			printf("\n User does not have SEND Access...");
    		}
    	}
    	else
    	{
    		printf("\n Something wrong with Happened...");
    	}

    }
    else if(mtype == SECURE)
    {

        printf("\nEnter name of Mailbox:");
        scanf("%s", name);

        int mailbox_id;
        // -3: for MLIST search...
        mailbox_id=sys_mlist_search_name(name,-1,SEARCH,-3, MID);

        if(mailbox_id == -1)
        {
        	//No Such mailbox Exists...
        	printf("\n No Such SECURE Mailbox Exists:");
        }
        else
        {
        	// ACCESS TO SECURE MAILBOX....
        	int chk1;
        	/*
        	 * mailbox_id= it is the mailbox MID here...identify the sender mailbox.
        	 * */
        	chk1=sys_msearch(sendID, SEARCH, mailbox_id);
        	if(chk1 == mailbox_id)
        	{
        		int chk2;
        		chk2=sys_deny_access(sendID, mailbox_id, SEND_BIT, CHKING);
        		if(chk2 == 1)
        		{
        			//SEND Bit is ON...
        			char recvID[50];
    	       	    bzero(recvID,50);
        			printf("\n Enter Receiver's ID for message:(Only 3/msg)");
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
        			    status = sys_mdeposite(sendID, recvID, msg, mailbox_id);

        			    if(status == -1)
        			    {
        			    	printf("\n Mailbox Overflow....System Halted");
        			    	printf("\n Goto Admin calls to clear it...");
        			    }
        			    else if(status == 1)
        			    {
        			    	printf("\n Status:Deposite_msg : %d\n", status);
        			    }
        			    else
        			    {
        			    	printf("\n Some error depositing Message ...");
        			    }
        			}
        		}
        		else if(chk2 == 0)
        		{
        			//SEND Bit is not set...OFF
        			printf("\n Sender ID does not have SEND Access...");
        		}
        	}
        	else
        	{
        		printf("\n No Such User for mailbox=%d",sendID);
        	}
        }
    }

}

void create_owner(int regnum)
{
	int owner_id, value=0, ret;
	int owner_mailbox_id =-2;

	printf("\n Enter New Owner ID:");
	scanf("%d", &owner_id);

	/*Register for Owner
	 * */
	ret = sys_msender(owner_id, REGISTER, -2);

	printf("\n Owner Register: Status=%d", ret);
	if(ret == 1)
	{
		printf("\nDuplicate: Owner already there...");
	}
	else if(ret == 0)
	{
		printf("\n Owner registered=%d...", owner_id);
	}
}

int change_prive()
{
	int ownID;
	printf("Enter Owner id:");
	scanf("%d", &ownID);

	/*
	 * owner list search OWNER: regnum=SEARCH & mailbox_id= -2..for Owner_list
	 * */
	int val;
	val = sys_msearch(ownID, SEARCH, -2);

	if(val == FOUND)
	{
		//owner id found...
		printf("\n Owner Found..changing Permissions");
		int count=0, i=0;
		char perm[5], *src, temp[5];

		bzero(temp,5);

		printf("\n Enter Permission:");
		scanf("%s", perm);
		strcat(perm, "\0");

		src = perm;
		strcat(src, "\0");

		/* c=99
		 * s=115
		 * r=114
		 * - = 45
		 * */
		while(src[i] != '\0')
		{
			if(src[i] == 99 || src[i]== 115 || src[i]== 114 || src[i]== 45)
			{
				count++;
			}
			i++;
		}
		if(count == 3)
		{
			int ret;
			printf("\n Perm to be set are: %s", perm);
			//mailbox_id = -2
			ret = sys_changep(perm, ownID, -2);
			if(ret == 1)
				printf("\n Permission updated Status:%d", ret);
		}
		else
		{
			//Invalid permissions assigned...
			printf("\n Owner permission can't be changed:");
		}
	}
	else
	{
		printf("\n Owner does not exists...");
	}
	return val;
}

void super_work()
{
	int status, choice, menu=1;

	while(menu)
	{
		printf("\n 1. Create Owner:");
		printf("\n 2. Change privileges of Owner");
		printf("\n 3. Display Owner Snap & Privileges");
		printf("\n 4. Delete Owner");
		printf("\n 5. Garbage & Halt recover");
		printf("\n 0. Back to main Menu..(Any choice)");
		printf("\n Enter Choice;");
		scanf("%d", &choice);

		if(choice == 1)
		{
			create_owner(REGISTER);
		}
		else if(choice == 2)
		{
			status = change_prive();
			printf("\n Privileges Updated:%d", status);
		}
		else if(choice == 3)
		{
			snapshot(OWNER_LIST);
		}
		else if(choice == 4)
		{
			delete_user(OWNER);
		}
		else
		{
			//Any other choice...go-back
			fflush(stdout);
			menu=0;
		}
	}
}

int validate_privileg(int ownID, int val)
{
	int i,j, ret=0;
	char temp[2];
	bzero(temp,2);

	if(val == CM)  // check for create_mailbox permission...
	{
		//send char "c" to owner list...
		strcat(temp,"c");
		ret = sys_msearch(ownID, CM, -2);  //-2 is mailbox-ID, 10=CM ID as regnum
	}
	else if(val == RW)
	{
		//send char "c" to owner list...
		strcat(temp,"s");
		ret = sys_msearch(ownID, RW, -2);  //-2 is mailbox-ID, 10=CM ID as regnum

	}
	else if(val == RM)
	{
		//send char "c" to owner list...
		strcat(temp,"r");
		ret = sys_msearch(ownID, RM, -2);  //-2 is mailbox-ID, 10=CM ID as regnum

	}

	return ret;
}

int mailbox_sender_recv(int regnum)
{
	int ownID, i, j, ret, retID;
	char mname[10];

	printf("\n ENter Owner ID:");
	scanf("%d", &ownID);

	ret = sys_msearch(ownID, SEARCH, -2);
	if(ret == FOUND)
	{
		printf("\n Name of Mailbox to which assign sender & receiver:");
		scanf("%s", mname);
		strcat(mname, "\0");
		// -3: mailbox_id for mlist
		retID = sys_mlist_search_name(mname, ownID,SEARCH, -3, MID);

		if(retID == -1)
		{
			printf("\n Mailbox Name is invalid...No Such Mailbox exists...");
			return retID;
		}
		else if(retID == -2)
		{
			printf("\n Name of mailbox is invalid against that Owner...");
			return retID;
		}
		else
		{
			//Found the mailbox id on Name of Mailbox...
			printf("\n Found the mailbox id on Name of Mailbox...%d",retID);
			register_user(retID);
		}

	}
	else
	{
		printf("\n Owner does not exists:");
		return ret;
	}
	return retID;
}

void mailbox_create_public()
{
	char name[100];
	strcpy(name,"public1");

	int status;
	status=sys_mlist_search_name(name,-1,SEARCH,-3,MID);

	if(status >= 0)
	{
		//Deny creation...
		printf("\n public Mailbox already there...Not allowed to create...");
	}
	else
	{
		//Allow to create Public...
		int ownID;
		printf("Enter Your Owner id:");
		scanf("%d", &ownID);

		int val;
		val = sys_msearch(ownID, SEARCH, -2);

		if(val == NOT_FOUND)
		{
			//owner id found...
			printf("\n You Are not the Owner:");
		}
		else if(val == FOUND)
		{
			// found...Proceed

			int check_perm;
			check_perm = validate_privileg(ownID,CM);
			printf("\n PERM found status:%d", check_perm);

			if(check_perm == NOT_FOUND)
			{
				// Don't have access...
				printf("\n You don't have access...");
			}
			else if(check_perm == FOUND )
			{
				int type=PUBLIC;

				if(type == PUBLIC)   //PUBLIC  Mailbox...
				{
					/*
					 * check for valid Name in list & owner ID in owners List
					 * */

			    	int chk1;
			    	chk1=sys_mlist_search_name(name,-1, SEARCH, -3, TYP);
			    	// -3 : mailbox_id for MLIST....
			    	if(chk1 == -1)
			    	{
			    		int no_of_sender=0, no_of_recv=0;

						int ret;
						ret=sys_mcreate(ownID, name, no_of_sender, no_of_recv, type );
						printf("\n mailbox Creation Success=%d", ret);
			    	}
			    	else
			    	{
			    		printf("\n Duplicate/NOT Found mailbox name:-ID= %d", chk1);
			    	}
				}
			} //CM Loop validation ends here.....
		}
	}
}


int denyuser(int flag)
{
	int i, userid, choice, ret=0, ownerid=-1;
	char name[10];

	if(flag == PUBLIC_LIST)
	{
		strcpy(name,"public1");
	}
	else
	{

		printf("\n Enter Owner ID:");
		scanf("%d", &ownerid);

		int check;
		//-2: mailbox_id
		check=sys_msender(ownerid,SEARCH,-2);
		if(check != 0)
		{
			printf("\n Duplicate owner or does not exists...");
			return -1;
		}
		else
		{
			printf("\n Enter Name of Mailbox:");
			scanf("%s", name);
		}
	}

	int mailbox_id;
	mailbox_id=sys_mlist_search_name(name, ownerid, SEARCH,-3,MID);

	if(mailbox_id != -1 )
	{

		printf("\n Enter user_ID");
		scanf("%d", &userid);

		int check=sys_msearch(userid, SEARCH, mailbox_id);
		if(check == mailbox_id)
		{
			//User ID exists for that mailbox...
			printf("\n Select Access removal:");
			printf("\n 1. DENY Send  2. DENy Receive  3.Update Access ");
			scanf("%d", & choice);
			if(choice == SEND_BIT)
			{
				ret=sys_deny_access(userid, mailbox_id, SEND_BIT, MAKING);
				printf("SEND Access removed=%d",ret);
			}
			else if(choice == RECV_BIT)
			{
				ret=sys_deny_access(userid, mailbox_id, RECV_BIT, MAKING);
				printf("RECEIVE Access removed=%d",ret);
			}
			else if(choice == 3)
			{
				int alow;
				printf("\n 1. SEND ACCESS  2. RECV ACCESS");
				scanf("%d",&alow);

				if(alow == 1)
				{
					ret=sys_deny_access(userid, mailbox_id, SEND_BIT, UPDT);
					printf("SEND Access Updated=%d",ret);
				}
				else if(alow == 2)
				{
					ret=sys_deny_access(userid, mailbox_id, RECV_BIT, UPDT);
					printf("RECEIVE Access Updated=%d",ret);
				}
				else
					printf("\n Invalid access choice...");
			}
			else
				printf("\n Wrong choice of access removal...");
		}
		else
		{
			printf("\n No Such User for this Mailbox=%s", name);
		}
	}
	else
	{
		printf("\n No such mailbox exists:");
	}
	return ret;
}


void mailbox_create()
{
	/*
	 * Check for valid
	 * */
	int ownID;
	printf("Enter Your Owner id:");
	scanf("%d", &ownID);

	/*
	 * owner list search OWNER: regnum=SEARCH & mailbox_id= -2
	 * */
	int val;
	val = sys_msearch(ownID, SEARCH, -2);

	if(val == NOT_FOUND)
	{
		//owner id found...
		printf("\n You Are not the Owner:");
	}
	else if(val == FOUND)
	{
		// found...Proceed

		int check_perm;
		check_perm = validate_privileg(ownID,CM);
		printf("\n PERM found status:%d", check_perm);

		if(check_perm == NOT_FOUND)
		{
			// Don't have access...
			printf("\n You don't have access...");
		}
		else if(check_perm == FOUND )
		{
			int type=SECURE;

			if(type == PUBLIC || type == SECURE)   //PUBLIC - SECURE Mailbox...
			{

				char name[100];
		   	    printf("\n Enter mailbox Name :");
		    	scanf (" %[^\n]%*c", name);

				/*
				 * check for valid Name in list & owner ID in owners List
				 * */

		    	int chk1;
		    	chk1=sys_mlist_search_name(name, -1,SEARCH, -3, MID);
		    	// -3 : mailbox_id for MLIST....
		    	if(chk1 == -1)
		    	{
		    		int no_of_sender=0, no_of_recv=0;

					int ret;
					ret=sys_mcreate(ownID, name, no_of_sender, no_of_recv, type );
					printf("\n mailbox Creation Success=%d", ret);

		    	}
		    	else
		    	{
		    		printf("\n Duplicate mailbox name:-ID= %d", chk1);
		    	}
			}
			else
			{
				printf("\n Invalid type of mailbox:");
			}

		} //CM Loop validation ends here.....
	}
}
void garbage_mailbox(int flag)
{
	int ret, val;
	char name[10];

	if(flag >= 0)
	{
		printf("\n Owner is deleting mailbox...");

		ret=sys_clear_mailbox(-1, flag);
		printf("\n Mailbox clear by Owner:%d", ret);
	}
	else
	{
		printf("\n 1. Owner List delete, \n2. User List delete");
		printf("\n Mailbox delete ");
		scanf("\n %d", &val);

		if(val == 1)
		{
			ret= sys_clear_mailbox(val,-1);
			printf("\n Garbage Collected Sender...");
		}

		else if(val == 2)
		{
			ret= sys_clear_mailbox(val,-1);
			printf("\n Garbage Collected Receiver...");
		}
		else if(val == -1)
		{
			//printf("\n All Mails Gone......");
			printf("\n Enter name of mailbox to delete:");
			scanf("\n %s", name);

			int mailbox_id;
			mailbox_id=sys_mlist_search_name(name,-1,SEARCH,-3,MID);
			if(mailbox_id >= 0)
			{
				sys_clear_mailbox(val, mailbox_id);
			}
		}

	}

}

void owner_task()
{
	int choice, menu=1, ret;
	char name[10];
	bzero(name, 10);

	while(menu)
	{
		fflush(stdout);
		printf(" \n Enjoy Being OWNER...\n");
		printf("1. Create New Mailbox (default Secure) \n");
		printf("2. Register Senders & Receivers \n");
		printf("3. Change privileges of User \n");
		printf("4. Empty Mailbox \n");
		printf("5. Display My Info \n");
		printf("0. Back to main menu \n");

		printf("Enter Choice:");
		scanf("%d", &choice);

		if(choice == 1)
		{
			mailbox_create();
		}
		else if(choice == 2)
		{
			mailbox_sender_recv(REGISTER);
		}
		else if(choice == 3)
		{
			ret = denyuser(USER_LIST);
			printf("User denied by Owner status=%d", ret);
		}
		else if(choice == 4)
		{
			int ownid;
			char name[10];

			printf("\n Enter Owner ID:");
			scanf("%d", &ownid);

			printf("\n Enter name of mailbox:");
			scanf("%s", name);

			int status;
			status=sys_mlist_search_name(name,ownid,SEARCH,-3,MID);
			if(status < 0)
			{
				printf("\n no such mailbox for that owner");
			}
			else
			{
				garbage_mailbox(status);
			}
		}
		else if(choice == 5)
		{
			//display complete mailbox and sender recv lists...
			int ownID, mailbox_id;
			char name[10];

			printf("\n Enter OwnerID:");
			scanf("%d", &ownID);

			printf("\n Enter name of mailbox:");
			scanf("%s", name);

			mailbox_id=sys_mlist_search_name(name,ownID,SEARCH,-3,MID);

			if(mailbox_id < 0)
			{
				printf("\nNo Such Owner Exists...call SuperUser.");
			}
			else
			{
				snapshot(mailbox_id);
			}
		}
		else
		{
			fflush(stdout);
			menu=0;
		}
	}
}


/*
int recover_halt()
{
	int ret;
	char receiver_data[100];
	bzero(receiver_data, 100);

	ret= sys_recoverhalt(receiver_data);
	printf("Receiver-Message List=%s", receiver_data);
	return ret;
}
*/

int main(int argc, char **argv)
{
	int input=-1, ret;
	int tmp;

	tmp=mailbox_init();

	printf("\n welcome to Mailbox IPC=%d", tmp);

	while(1)
	{

		printf("\n-----Available Operations---\n");
		printf("1. Create Public Mailbox \n");
		printf("2. Deposit message \n");
		printf("3. SnapShot Mailbox (public Mailbox) \n");
		printf("4. ALLOW/DENY Send/Receive access (Public mailbox) \n");
		printf("5. Remove User from Public List \n");
		printf("6. Garbage Mailbox \n");
		printf("7. Owner Task \n");
		printf("8. SuperUser Work \n");
        printf("0. Exit the Program \n");
		printf("Enter your choice : ");
		scanf(" %d",&input);
        switch(input)
        {
        	case 1:
        			mailbox_create_public();
        			break;
        	case 2:
        			deposite_mesg();
        			break;
        	case 3:
        			fflush(stdout);
        			snapshot(PUBLIC_LIST);
        			break;
        	case 4:
        			ret=denyuser(PUBLIC_LIST);
        			break;
        	case 5:
        			delete_user(USER);
        			break;
		    case 6:
		    		garbage_mailbox(-1);
		    		//recover_halt();
		    		break;
		    case 7:
        			fflush(stdout);
        			owner_task();
		            break;
		    case 8:
		    		super_work();
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
