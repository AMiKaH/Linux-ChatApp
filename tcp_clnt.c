/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		tcp_clnt.c - A simple TCP client program.
--
--	PROGRAM:		Linux Chat Client
--
--	FUNCTIONS:		Berkeley Socket API
--
--	DATE:			April 8, 2020
--
--	REVISIONS:	
--
--
--	DESIGNERS:	    Jason Nguyen, Amir Kbah
--
--				
--	PROGRAMMER:	    Jason Nguyen, Amir Kbah
--
--	NOTES:
--	The program will establish a TCP connection to a user specifed server.
-- The server can be specified using a fully qualified domain name or and
--	IP address. After the connection has been established the user will be
-- able to send messages to all clients connected to the same server.
-- Messages will be received from server, but only the messages from other clients.
---------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h> 
#include <string.h>

#define SERVER_TCP_PORT		7000	// Default port
#define BUFLEN			255  	// Buffer length

void *readThread(void *vargp);

int n, bytes_to_read;
int sd, port;
struct hostent	*hp;
struct sockaddr_in server;
char  *host, *bp, rbuf[BUFLEN], sbuf[BUFLEN], **pptr;
char str[16];
FILE *fptemp;
FILE *fpdump;
int pid;
char strtemp[20];
char strdump[20];
int save;

/*---------------------------------------------------------------------------------------
--	FUNCTION: main
--
--	DATE:			April 8, 2020
--
--	REVISIONS:		
--
--
--	DESIGNERS:	    Jason Nguyen, Amir Kbah
--
--				
--	PROGRAMMER:	    Jason Nguyen, Amir Kbah
--
--  INTERFACE:  int main(int argc, char **argv)
--                  int argc: the number of arguments
--                  char **argv: an array containing the ip address and port to connect to
--
--  RETURNS:    int: 0 on success
--
--	NOTES:
--	Connects to a server and begins sending and receiving messages from it. The reading is
--  on a separate thread. To stop type 'q' and enter. A dump file of the chat log can be
--  saved if the user chooses to. The file name will be [pid]dump.txt
---------------------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
    pid = getpid();
    char mypid[6];
    sprintf(mypid, "%d", pid);
    strcpy(strtemp, mypid);
    strcat(strtemp, ".txt");
    fptemp = fopen(strtemp, "w");
	switch(argc)
	{
		case 2:
			host =	argv[1];	// Host name
			port =	SERVER_TCP_PORT;
		break;
		case 3:
			host =	argv[1];
			port =	atoi(argv[2]);	// User specified port
		break;
		default:
			fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
			exit(1);
	}

	// Create the socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Cannot create socket");
		exit(1);
	}
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if ((hp = gethostbyname(host)) == NULL)
	{
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}
	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

	// Connecting to the server
	if (connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		exit(1);
	}
	printf("Connected:    Server Name: %s\n", hp->h_name);
	pptr = hp->h_addr_list;
	printf("\t\tIP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));
	

	// Transmit data through the socket

	bp = rbuf;
	bytes_to_read = BUFLEN;

    	pthread_t thread_id; 
    	pthread_create(&thread_id, NULL, readThread, NULL);

	// client makes repeated calls to recv until no more data is expected to arrive.
	n = 0;
	while (1)
	{
		fgets (sbuf, BUFLEN, stdin);
		send (sd, sbuf, BUFLEN, 0);
		if(sbuf[0] == 'q' && sbuf[1] == '\n') {
		    printf("Would you like to save the chat log? [y/n]\n");
		    fgets (sbuf, BUFLEN, stdin);
		    if (sbuf[0] == 'y')
		        save = 1;
		    else
		        save = 0;
		    break;
		}
		fflush(stdout);

		fprintf(fptemp, "%s", sbuf);
	}

	fclose(fptemp);
	close (sd);

	if (save)
	{
	fptemp = fopen(strtemp, "r");

	strcpy(strdump, mypid);
	strcat(strdump, "dump.txt");
	fpdump = fopen(strdump, "w");

	char c;
	while ((c = fgetc(fptemp)) != EOF) {
	    fputc(c, fpdump);
	}

	fclose(fptemp);
	fclose(fpdump);
	}
	remove(strtemp);
	return (0);
}

/*---------------------------------------------------------------------------------------
--	FUNCTION: readThread
--
--	DATE:			April 8, 2020
--
--	REVISIONS:		
--
--
--	DESIGNERS:	    Amir Kbah
--
--				
--	PROGRAMMER:	    Amir Kbah
--
--  INTERFACE:  void *readThread(void *vargp)
--
--	NOTES:
--	Reads data from server using the socket descriptor and prints it out.
--
---------------------------------------------------------------------------------------*/
void *readThread(void *vargp) 
{ 
    while (1)
	{
		if ((n = recv (sd, bp, 255, 0)) > 0) {
			printf ("%s", rbuf);
			fprintf(fptemp, "%s", rbuf);
			
		} else if (n == 0) {
			continue;
		}
	}

    return NULL; 
} 
