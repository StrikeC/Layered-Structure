#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include "structs.h"

// handle error
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

// the thread function to receive packets from the data link layer and display the messages
void * rcvmsg (int threadsockfd)
{
	/*add codes for local varialbes*/

	while (1)
	{
	/*add codes to read a packet from threadsockfd and display it to the screen*/
		...

	}
	return NULL;
}


int main(int argc, char *argv[])
{
	int sockfd, portno, ret;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	char buffer[256];

	// check number of aruguments
    if (argc < 4) {
       fprintf(stderr,"usage %s data_add data_port nickname\n", argv[0]);
       exit(0);
    }

	/*add codes to connect to the data link layer*/
    // create socket
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket!");
	// retrieve server name
    server = gethostbyname(argv[1]);
    if (server == NULL) 
    {
        fprintf(stderr,"ERROR, no such host!\n");
        exit(0);
    }
	// fill in server address structure
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
	// connect server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting!");
    printf("Ready to communicate\n");

	/*creat a thread to receive packets from the data link layer*/
	pthread_t pth;	// this is our thread identifier
	pthread_create(&pth,NULL,rcvmsg,sockfd);

	/* the main function will receive messages from keyboard and send packets to the data link layer*/
	while (1)
	{
		/*add codes to receive a message from keyboard, wrap it into a packet and send it to the data link layer*/
		// receive message from keyboard
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);

		/*if the meassge is "EXIT"*/
		if (strcmp (buffer, "EXIT\n")==0)
		{
			pthread_cancel(pth); //kill the child thread
			close(sockfd); // close socket
			return 0;	//terminate
		}
	}

}
