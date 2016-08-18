Execute following commands:

1. cp project3.c receiver.c /root
2. cp mail_ipc.c mail_ipc.h mailboxq.c table.c proto.h  /usr/src/servers/pm
3. cp call_nr.h /usr/src/include/minix
4. cp project2.h /usr/include
5. goto /usr/src/servers/pm
	cat Makefile
	add line 'mail_ipc.c into" it.

====================

To execute :

Terminal-1:
1. goto /root
2. cc project3.c -o project3
3. ./project3

terminal-2:
1. cc receiver.c -o receiver
2. ./receiver


and follow the options given...