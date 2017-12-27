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

// struct for threads
struct threadSocArg {
  int index;
};

/* functions */
// convert string to frame
frame frameToRead(char frameIn_s[269]);

// connection thread
void * onesocket (void * arg)
{
	char buffer[269]; // read buffer
	frame bufferFrame; // read buffer for frame
  struct threadSocArg *thread_soc_arg = (struct threadSocArg*)arg;
  int sockfd = clientlist[thread_soc_arg->index];
	int theOtherSide_sockfd; // write socket
	int ret; // for return value

  if (thread_soc_arg->index == 0)
    theOtherSide_sockfd = clientlist[1];
  else if (thread_soc_arg->index == 1)
    theOtherSide_sockfd = clientlist[0];

	// read/write loop
	while (1)
	{
		// empty buffer
		bzero(buffer,269);
		bzero(bufferFrame.my_packet.nickname,10);
		bzero(bufferFrame.my_packet.message,256);
		// read from client
    ret = read(sockfd, buffer, 268);
    if (ret < 0)
      error("ERROR reading from socket!");
		else if (ret == 0) //indicate that client exit connection
      break;
		// convert buffer to farme
		bufferFrame = frameToRead(buffer);
		// print nickename of client
		printf("Recieved a frame from machine: %s\n", bufferFrame.my_packet.nickname);
		// when client send EXIT, the thread returns
		if (strcmp(bufferFrame.my_packet.message,"EXIT\n") == 0)
			break;
		// reply to the other client
        ret = write(theOtherSide_sockfd, buffer, strlen(buffer));
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
	socklen_t clilen = sizeof(cli_addr);
	pthread_t threadlist[2];
  struct threadSocArg thread_soc_arg;

	// check the number of arguments
     if (argc < 2)
	 {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

	/*create socket and listen to it */
	// create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
        error("ERROR opening socket!");
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

	// accept loop
	for (int i=0;i<2;i=i+1) /*only accept two requests*/
	{
		/*accept a request from the data link layer*/
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
			error("ERROR on accept");
		else
			printf("create new socket: %d\n", newsockfd);
		/* store the new socket into clientlist*/
		clientlist[i] = newsockfd;
    thread_soc_arg.index = i;
		/*creat a thread to take care of the new connection*/
		pthread_t pth;	/* this is the thread identifier*/
		pthread_create(&pth, NULL, onesocket, &thread_soc_arg);
		threadlist[i]=pth; /*save the thread identifier into an array*/
	}

	close(sockfd); /*so that wire will not accept further connection request*/
	pthread_join(threadlist[0], NULL);
	pthread_join(threadlist[1], NULL); /* the main function will not terminated untill both threads finished*/
	return 0;

}
