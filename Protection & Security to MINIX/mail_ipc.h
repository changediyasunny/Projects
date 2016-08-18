#define MAX_RECV 10
#define MAX_SEND 10
#define MAX_USER 10
#define MAX_MESG_LEN 100
#define MAX_MSG_NO 16
#define MAX_MAILBOX_NAME 100
#define MAX_MAILBOX 10
#define MAX_PUB_USERS 20

#define PUBLIC 0
#define SECURE 1
#define REGISTER 1
#define SEARCH 0
#define ALLOW 1
#define BLOCK -1

#define DENY -7
#define SEND_BIT 1
#define RECV_BIT 2
#define CHKING 1
#define MAKING 2
#define PUBCHK -1
#define UPDT -2

#define MLIST -3
#define OWNER_DATA -2

#define CM_FOUND 10
#define RW_FOUND 11
#define RM_FOUND 12
#define ON 1
#define OFF 0

#define FOUND 1
#define NOT_FOUND -1

#define OWNER_LIST -1
#define USER_LIST  0
#define PUBLIC_LIST -2

#define	MESSAGING_SUCCESS 0
#define	DUPLICATE_SENDER  4
#define	DUPLICATE_RECEIVER 5
#define	MSG_LEN_OVERFLOW_ERROR 6
#define	MAILBOX_OVERFLOW 8
#define	MAILBOX_UNDERFLOW 9

int mailbx_num;
int no_of_owner;
int owner_list[10];


int mail_insert(char data[], int no_of_recv, int destrec[], int mailbox_id);
int mail_receive(char *data[], int recid, int mailbox_id);
int mail_init(int ownID, char name[10], int no_of_sender, int no_of_recv, int type);
int add_user_list(int sid, int regnum, int mailbox_id);
int add_recv_list(int rid, int regnum, int mailbox_id);
int add_owner_list(int ownerID, int regnum, int mailbox_id);
void mailbox_init();
int change_perm(int ID, int mailbox_id, char *perm);
int search_list(char name[10], int id, int search, int mailbox_id, int flag);
void make_mailbox(int pos, int mailbox_id);
void delete_user(int pos, int mailbox_id);
int remove_user(int user_id, int mailbox_id, int user_flag);
int deny_access(int userid, int mailbox_id, int bit, int flag);

struct mailbox_name_id_list
{
	char mname[MAX_MAILBOX_NAME];
	int MID;
	int ownid;
	int type;
};

struct mailbox_name_id_list mlist[MAX_MAILBOX];

struct owner_data
{
	int OID;
	char priv[5];
};

struct owner_data owner[10];


typedef struct user
{
	int user_id;
	int send_bit;
	int recv_bit;
	int access;
}user;

typedef struct msgpack
{
	char mesg[MAX_MESG_LEN];
	int reclist[10];
	int sender_proc_id;
	int rec_proc_id;
	int num_of_read_completed;
	// If 1 it means that the message has been deleted
	int deletedFlag;

}msgpack;

struct mailbox_node
{
	msgpack msgList[MAX_MSG_NO];
	user user_list[MAX_USER];

	int mailbox_type;  //1--secure, 0--public
	char name[MAX_MAILBOX_NAME];
	int owner_id;
	int recv_list[MAX_RECV];
	int sender_list[MAX_SEND];
	int mail_cnt;      //count of mail check in mailbox; mail_cnt <16
	int no_of_sender; //maintain sender list
	int no_of_recv;  // maintain receiver list
	int total_user;

};


struct mailbox_node *mailbox[MAX_MAILBOX];


