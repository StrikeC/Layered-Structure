#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> // The file netdb.h defines the structure hostent, which will be used below.
#include <strings.h>

// handle error
void error(const char *msg)
{
    perror(msg);
    exit(0);
}


int main(int argc, char *argv[])
{
    int sockfd, portno, ret;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
	
    // check argument number
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
	
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
    printf("Waiting for connecting...\n");
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting!");
    printf("Server connected.\n");
    // communication loop
    while(1)
    {
        printf("Please enter the message: ");
        // send message to the server
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        ret = write(sockfd,buffer,strlen(buffer));
        if (ret < 0) 
            error("ERROR writing to socket!");
        // EXIT condition
        if (strcmp(buffer,"EXIT\n") == 0)
            break;
        // retrieve the acknowledgment
        bzero(buffer,256);
        ret = read(sockfd,buffer,255);
        if (ret < 0) 
            error("ERROR reading from socket!");
        else if (ret == 0)
        {
            printf("Server has exited!\n");
            break;
        }
        printf("Server says: %s\n", buffer);
    }
    // close socket
    close(sockfd);
    return 0;
}