#define MAX_RECV 5
#define MAX_SEND 5
#define MAX_USER 10
#define MAX_MESG_LEN 100

#define	MESSAGING_SUCCESS 0
#define	DUPLICATE_SENDER 4
#define	DUPLICATE_RECEIVER 5
#define	MSG_LEN_OVERFLOW_ERROR 6
#define	MAILBOX_OVERFLOW 8
#define	MAILBOX_UNDERFLOW 9


int mail_cnt;      //count of mail check in mailbox; mail_cnt <16
int no_of_sender; //maintain sender list
int no_of_recv;  // maintain receiver list
int total_user;

int sender_list[5];
int recv_list[5];
int user_list[MAX_USER];

int mail_insert(char data[], int uid, int destrec[], int no_of_recv);
int mail_receive(char *data[], int recid);
void mail_init();
int add_sender_list(int sid, int regnum);
int add_recv_list(int rid, int regnum);
void make_mailbox(int pos);
int add_mail_list(int sid, int regnum);


struct node
{
	int recv_id;
	int pending_mesg;
};

struct node snapshot[20];

struct msgpack
{
	char mesg[MAX_MESG_LEN];
	int reclist[5];
	//List of receivers who can read the message
	int sender_proc_id;
	int rec_proc_id;
	int num_of_read_completed;
	// If 1 it means that the message has been deleted
	int deletedFlag;

};

typedef struct my_mailbox
{
	struct msgpack msgList;
	struct my_mailbox *next;
}my_mailbox;

my_mailbox mailbox[16];

