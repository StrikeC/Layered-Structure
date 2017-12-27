#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "structs.h"
#include <arpa/inet.h> 
#include <strings.h>

// handle error
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// list for storing clint sockets
int clientlist[2]; 

/* functions */
// convert string to frame
frame frameToRead(char frameIn_s[269]);

// exit server thread
void *thread_exit(void *arg)
{
    char buffer[8];
    bzero(buffer, 8);
    while(1)
    {
        fgets(buffer, 7, stdin);
        if (strcmp(buffer,"EXIT\n") == 0)
            exit(0);
    }
    return NULL;
}

// connection thread  
void *onesocket (int pthreadArg[2])
{
	char buffer[269]; // read buffer
	frame bufferFrame; // read buffer for frame
	int sockfd = clientlist[pthreadArg[0]]; // read socket
	int pCli_sockfd = clientlist[pthreadArg[1]]; // write socket
	int ret; // for return value
	
	// read/write loop
	while ()
	{
		// empty buffer
		bzero(buffer,269);
		bzero(bufferFrame.my_packet.nickename,10);
		bzero(bufferFrame.my_packet.message,256);
		// read from client
        ret = read(sockfd,buffer,268);
        if (ret < 0) 
            error("ERROR reading from socket!");
		else if (ret == 0) //indicate that client exit connection
            break;
		// convert buffer to farme
		bufferFrame = frameToRead(buffer);
		// print nickename of client
		printf("Recieved a frame from machine: %s\n", bufferFrame.my_packet.nickname);
		// when client send EXIT\n, the thread returns
		if (strcmp(bufferFrame.my_packet.message,"EXIT\n") == 0)
			break;
		// reply to the other client
        ret = write(pCli_sockfd,buffer,268);
        if (ret < 0) 
            error("ERROR writing to socket!");
		else
			printf("Sending it to machine on the other side...\n");
	}
	// close up
    printf("Machine: %s has exited!\n", bufferFrame.my_packet.nickname);
    close(sockfd);
    return NULL;
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	struct sockaddr_in serv_addr, cli_addr;
	int threadArg[2];
	socklen_t clilen = sizeof(cli_addr);
	pthread_t threadlist[2];
	
	// check the number of arguments
     if (argc < 2) 
	 {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
	// create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
        error("ERROR opening socket!");
	// reuse ports
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        error("SO_REUSEADDR setting failed!");
	// fill in server address structure
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
	// bind socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding!");
	// listen
    listen(sockfd, 5);
	// create a thread for manual server exit
    pthread_t pth;
    pthread_create(&pth, NULL, thread_exit, NULL);
	// accept loop
	for (i=0;i<2;i=i+1) /*only accept two requests*/
	{
		/*accept a request from the data link layer*/
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
			error("ERROR on accept");
		else
			printf("create new socket: %d\n", newsockfd);
		/* store the new socket into clientlist*/
		clientlist[i]=newsockfd;
		threadArg[0] = i;
		threadArg[1] = 0 - (i - 1);
		/*creat a thread to take care of the new connection*/
		pthread_t pth;	/* this is the thread identifier*/
		pthread_create(&pth,NULL,onesocket,threadArg);
		threadlist[i]=pth; /*save the thread identifier into an array*/ 
	}
	
	close(sockfd); /*so that wire will not accept further connection request*/
	pthread_join(threadlist[0],NULL);
	pthread_join(threadlist[1],NULL); /* the main function will not terminated untill both threads finished*/
	return 0;

}
