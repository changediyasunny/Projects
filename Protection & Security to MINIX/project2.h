#include<lib.h>
#include<unistd.h>

#define USER 1
#define RECEIVER 2
#define OWNER 3
#define SUPERUSER 0

#define MLIST -3
#define OID -1
#define MID 0
#define TYP 1

#define OWNER_LIST -1
#define USER_LIST  0
#define PUBLIC_LIST -2

#define DENY -7
#define SEND_BIT 1
#define RECV_BIT 2
#define CHKING 1
#define MAKING 2
#define PUBCHK -1
#define UPDT -2

#define REGISTER 1
#define SEARCH 0

#define MB_NAME_ID_LIST -3
#define FOUND 1
#define NOT_FOUND -1

#define CM 10
#define RW 11
#define RM 12
#define PUBLIC 1
#define SECURE 0
#define ACCESS 1

int mailbox_init()
{
	message ms;
	printf("\n Project2.h file: mailbox init...");
	return (_syscall(PM_PROC_NR, MINIT, &ms));
}

int sys_mcreate(int ownID, char name[], int no_of_sender, int no_of_recv, int type)
{
	message ms;
	int ret;

	/*
	 * When assigning Message, Do not give space ! it will ruin your day !!1
	 * */

	ms.m7_p1=name;

	ms.m7_i1 = ownID;
	ms.m7_i2 = no_of_sender;
	ms.m7_i3 = no_of_recv;
	ms.m7_i4 = type;

	//MCREATE = 58

	ret = _syscall(PM_PROC_NR, MCREATE, &ms);

	printf("\n Project2.h..ret received");

	return ret;
}

int sys_deny_access(int userid, int mailbox_id, int bit, int flag)
{
	message ms;

	ms.m7_i1=userid;
	ms.m7_i2=mailbox_id;
	ms.m7_i3=bit;
	ms.m7_i4=flag;
	//110; DENYACCESS
	return( _syscall(PM_PROC_NR,110 ,&ms) );
}

int sys_changep(char *temp, int ownID, int mailbox_id)
{
	message ms;

	ms.m7_i1=ownID;
	ms.m7_i2=mailbox_id;
	ms.m7_p1=temp;
	// 97: CHANGEP
	return ( _syscall(PM_PROC_NR,97, &ms) );
}

int sys_remove_user(int user_id, int mailbox_id, int user_flag)
{
	int ret;
	message ms;

	ms.m7_i1=user_id;
	ms.m7_i2=mailbox_id;
	ms.m7_i3=user_flag;
	//105: REMUSER
	ret = _syscall(PM_PROC_NR,105,&ms);
	return ret;

}

int sys_msearch(int user_id, int regnum, int mailbox_id)
{
	int ret=0;
	message ms;

	ms.m7_i1=user_id;         //User ID
	ms.m7_i2=regnum;     // RegNum
	ms.m7_i3=mailbox_id; // mailboxID from name of mailbox...

	printf("\n Going to search Owner !");

	// 103: MSEARCH
	ret = _syscall(PM_PROC_NR,103,&ms);

	//printf("\n Project2.h: ret=%d", ret);
	return ret;
}

int sys_mlist_search_id(int id, int regnum, int mailbox_id)
{
	int ret;
	message ms;

	ms.m7_i2=regnum;     //SEARCH
	ms.m7_i3=mailbox_id; //-3
	ms.m7_i1=id;
	ms.m7_i4=-2;

	printf("\n Going to Map name & ID for mailbox !");

	// 103: MSEARCH
	ret = _syscall(PM_PROC_NR,103,&ms);
	return ret;
}


int sys_mlist_search_name(char name[10], int userid, int regnum, int mailbox_id, int flag)
{
	int ret;
	message ms;

	ms.m7_p1=name;       //name of mailbox

	ms.m7_i1=userid;         //-1 for no owner ID search
	ms.m7_i2=regnum;     //SEARCH
	ms.m7_i3=mailbox_id; //-3 for mlist mailbox
	ms.m7_i4=flag;       //to return MID=0 or OID=-1...

	printf("\n Going to Map name & ID for mailbox !");

	// 103: MSEARCH
	ret = _syscall(PM_PROC_NR,103,&ms);

	return ret;
}

int sys_msender(int sid, int regnum, int mailbox_id)
{
	message ms;

	ms.m7_i1 = sid;         //sender ID
	ms.m7_i2 = regnum;     // RegNum
	ms.m7_i3 = mailbox_id; // mailboxID from name of mailbox...

	printf("\n Registering User for ID=%d", ms.m7_i1);

	//.....MSENDER=108
	return( _syscall(PM_PROC_NR, 108, &ms) );
}

int sys_mreciver(int rid, int regnum, int mailbox_id)
{
	message ms;
	ms.m7_i1=regnum;
	ms.m7_i2 = rid;
	ms.m7_i3=mailbox_id;
	printf("\n calling mreceiver=109 for rid=%d", rid);
	//.....MRECEIVER=109
	return( _syscall(PM_PROC_NR, 109, &ms) );
}

int sys_mdeposite(int sendID, char recvID[], char msg[], int mailbox_id)
{
	int ret, i=0, no_of_recv=0, recv[10];
	message ms;
	char *token;

	token=strtok(recvID, ",");
	while(token != NULL)
	{
		recv[no_of_recv]=atoi(token);
		no_of_recv++;
		token=strtok(NULL,",");
	}

	ms.m7_i1=mailbox_id;
	ms.m7_i2=no_of_recv;

	ms.m7_i3=recv[0];
	ms.m7_i4=recv[1];
	ms.m7_i5=recv[2];

	ms.m7_p1=msg;

	// 69: USRSEND
	ret=  _syscall(PM_PROC_NR, USRSEND, &ms);

	return( ret );
}

int sys_mreceive(int recvID, char *msg, int mailbox_id)
{
	int ret;
	message ms;

	ms.m7_i1=mailbox_id;
	ms.m7_i2=recvID;
	ms.m7_p1=msg;

	// USRRECIVE=70
	ret=  _syscall(PM_PROC_NR, 70, &ms);

	return( ret );
}


int sys_snapshot(char *mails, char *user_list, int mailbox_id)
{
	int ret;
	message ms;

	ms.m7_i1=mailbox_id;
	ms.m7_p2=user_list;
	ms.m7_p1=mails;

	// SNAP=44
	ret = _syscall(PM_PROC_NR, 44, &ms);

	return ret;
}


int sys_clear_mailbox(int val, int mailbox_id)
{
	int ret=-1;

	message ms;
	ms.m7_i1=val;
	ms.m7_i2=mailbox_id;

	// 56: do_mgarbage
	ret= _syscall(PM_PROC_NR, 56, &ms);

	return ret;
}


int sys_recoverhalt(char *receiver_data)
{
	int ret;
	message ms;

	ms.m7_p1=receiver_data;
	// 35: do_halt_recover
	return ( _syscall(PM_PROC_NR, 35, &ms) );
}





