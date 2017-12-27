#include <stdio.h>
#include <stdlib.h>	// for IOs
#include <string.h>
#include <unistd.h>
#include <sys/types.h>	// for system calls
#include <sys/socket.h>	// for sockets
#include <netinet/in.h>	// for internet
#include <pthread.h>	// for thread
#include <arpa/inet.h>  // for inet_ntoa
#include <strings.h>

// handle error
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

// store the threadFunc arguments
struct threadSocArg
{
    int socket;
    struct sockaddr_in* pCli_addr;
};

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
void *thread_connection(void *arg)
{
    char buffer[256]; // read buffer
    struct threadSocArg *pThread_soc_arg = (struct threadSocArg*)arg; // read in arguments
    int sockfd = pThread_soc_arg->socket; // accepted socket
    char *pClient_IP = inet_ntoa(pThread_soc_arg->pCli_addr->sin_addr); // client IP
    int ret; // for return value
    printf("Client %s has connected.\n", pClient_IP);
    
	// when client send EXIT\n, the thread returns
    while (strcmp(buffer,"EXIT\n") != 0)
    {
        bzero(buffer,256); // empty buffer
        // read from client
        ret = read(sockfd, buffer, 255); 
        if (ret < 0) 
            error("ERROR reading from socket!");
		else if (ret == 0) //indicate that client exit connection
            break;
		// print message of client
        printf("Client %s says: %s", pClient_IP, buffer); // print message of client
        // reply to client
        ret = write(sockfd,"I got your message.",19);
        if (ret < 0) 
            error("ERROR writing to socket!");
    }
    // close up
    printf("Client %s has exited!\n", pClient_IP);
    close(sockfd);
    return NULL;
}


int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;
    struct threadSocArg thread_soc_arg;
    socklen_t clilen = sizeof(cli_addr);
	
    // check the number of arguments
    if (argc < 2) 
    {
        fprintf(stderr, "ERROR, no port provided!\n");
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
	// create a thread for server exit
    pthread_t pth;
    pthread_create(&pth, NULL, thread_exit, NULL);
	
    // accept loop
    printf("Waiting for clients...\n");
    while(1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd < 0)
            error("ERROR on accept");
        // create a thread for accepted client
        pthread_t pth;
        thread_soc_arg.socket = newsockfd;
        thread_soc_arg.pCli_addr = &cli_addr;
        pthread_create(&pth, NULL, thread_connection, &thread_soc_arg);
    }

    close(sockfd);
    return 0;
}