#include<lib.h>
#include<unistd.h>

int mailbox_init()
{
	message ms;
	printf("\n Project2.h file: mailbox init...");
	return (_syscall(PM_PROC_NR, MINIT, &ms));
}

int sys_msearch(int sid, int regnum)
{
	message ms;
	ms.m7_i1=sid;
	ms.m7_i2=regnum;

	printf("\n Searching for sender/ receiver...");
	// 58: MSEARCH...
	return(_syscall(PM_PROC_NR, 58, &ms));
}

int sys_msender(int sid, int regnum)
{
	message ms;

	ms.m7_i1 = sid;
	ms.m7_i2 = regnum;
	printf("\n Registering sender for sendID=%d", sid);

	//.....MSENDER=108
	return( _syscall(PM_PROC_NR, 108, &ms) );
}

int sys_mreciver(int rid, int regnum)
{
	message ms;
	ms.m7_i1=regnum;
	ms.m7_i2 = rid;
	printf("\n calling mreceiver=109 for rid=%d", rid);
	//.....MRECEIVER=109
	return( _syscall(PM_PROC_NR, 109, &ms) );
}

int sys_mdeposite(int sendID, char recvID[], char msg[])
{
	int ret, i=0, no_of_recv=0, recv[5];
	message ms;
	char *token;

	token=strtok(recvID, ",");
	while(token != NULL)
	{

		recv[no_of_recv]=atoi(token);
		no_of_recv++;
		token=strtok(NULL,",");
	}

	/*
	 * assign multiple recvs to m7 msg structure...
	 * ms.m7_i3, ms.m7_i4, ms.m7_i5
	 *
	 * */
	ms.m7_i1=sendID;
	ms.m7_i2=no_of_recv;

	ms.m7_i3=recv[0];
	ms.m7_i4=recv[1];
	ms.m7_i5=recv[2];

	ms.m7_p1=msg;

	// 69: USRSEND
	ret=  _syscall(PM_PROC_NR, USRSEND, &ms);

	return( ret );
}

int sys_mreceive(int recvID, char *msg)
{
	int ret;
	message ms;

	ms.m7_i2=recvID;
	ms.m7_p1=msg;

	// USRRECIVE=70
	ret=  _syscall(PM_PROC_NR, 70, &ms);

	return( ret );
}

int sys_snapshot(char *mails, char *user_list)
{
	int ret;
	message ms;

	ms.m7_p1=mails;
	ms.m7_p2=user_list;

	// SNAP=44
	ret = _syscall(PM_PROC_NR, 44, &ms);
	return ret;
}


int sys_clear_mailbox(int val)
{
	int ret=-1;

	message ms;
	ms.m7_i1=val;

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





