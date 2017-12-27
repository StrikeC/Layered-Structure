#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include "structs.h"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

/*global variables to be used in the threads*/
int network_layersockfd;//the socket through which the network layer is connected.
int wiresockfd; //the socket through which the wire is connected.

/* functions */
// convert string to packet
packet packetToRead(char packetIn_s[266]);
// convert packet to frame
frame packet2frame(packet packetIn, int seq_num, int type);
// convert frame to string
void frameToSend(frame frameIn_f, char frameOut_s[269]);
// convert string to frame
frame frameToRead(char frameIn_s[269]);
// convert frame to packet
packet frame2packet(frame frameIn);
// convert packet to string
void packetToSend(packet packetIn_p, char packetOut_s[266]);

/*the thread function that receives frames from the wire socket and sends packets to the network_layer */
void * rcvfromwiresend2network_layer (char *argv[])
{
	/*add codes to declare locals*/
	char pbuffer[269], pbufferPacket_str[266];
	packet pbufferPacket;
	frame pbufferFrame;
	int ret;

	while (1)
	{
		/*add codes receive a frame from wire*/
		// empty buffer
		bzero(pbuffer,269);
		bzero(pbufferPacket.nickname,10);
		bzero(pbufferPacket.message,256);
		bzero(pbufferFrame.my_packet.nickname,10);
		bzero(pbufferFrame.my_packet.message,256);
		bzero(pbufferPacket_str,266);
		// read from wire
		ret = read(wiresockfd, buffer, 268);
		if (ret < 0) 
            error("ERROR reading from socket!");
		else if (ret == 0) //indicate that client exit connection
            break;
		// convert buffer to frame
		pbufferFrame = frameToRead(buffer);
		// display
		printf("Received a packet from wire\n");
		printf("       Sequence Number: %d\n", pbufferFrame.seq_num);
		printf("       Frame Type: %\n", pbufferFrame.type);
			 
		/*add codes to send the included packet to the network layer*/	
		// convert frame to packet
		pbufferPacket = frame2packet(pbufferFrame);
		// convert packet to string
		packetToSend(pbufferPacket, pbufferPacket_str);
		// write to network layer
		ret = write(network_layersockfd, pbufferPacket_str, strlen(pbufferPacket_str));
		if (ret < 0) 
            error("ERROR writing to socket!");
		else
			printf("Sending the included packet to network_layer...\n");
	}
	return NULL;
}


int main(int argc, char *argv[])
{
	int sockfd, portno2wire, portno, ret, seq_num_count;
	struct sockaddr_in serv_addr2wire, serv_addr, cli_addr;
	struct hostent *server;
	socklen_t clilen = sizeof(cli_addr);
	char buffer[266], bufferFrame_str[269];
	packet bufferPacket;
	frame bufferFrame;

	// check numeber of arguments
    if (argc < 4) {
		fprintf(stderr,"Usage: %s  wire__IP  wire_port data_port\n",argv[0] );
        exit(1);
    }

	/* add codes to connect to the wire. Assign value to gobal varialbe wiresockfd */
	// create socket
    portno2wire = atoi(argv[2]);
    wiresockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (wiresockfd < 0) 
        error("ERROR opening socket!");
	// retrieve server name
    server = gethostbyname(argv[1]);
    if (server == NULL) 
    {
        fprintf(stderr,"ERROR, no such host!\n");
        exit(0);
    }
	// fill in server address structure
    bzero((char *) &serv_addr2wire, sizeof(serv_addr2wire));
    serv_addr2wire.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char *)&serv_addr2wire.sin_addr.s_addr, server->h_length);
    serv_addr2wire.sin_port = htons(portno2wire);
	// connect server
    if (connect(wiresockfd, (struct sockaddr *) &serv_addr2wire, sizeof(serv_addr2wire)) < 0) 
        error("ERROR connecting!");

	/*generate a new thread to receive frames from the wire and pass packets to the network layer */
	pthread_t wirepth;	// this is our thread identifier
	pthread_create(&wirepth,NULL,rcvfromwiresend2network_layer, NULL);

	
	/*add codes to create and listen to a socket that the network_layer will connect to. Assign value to global variable network_layersockfd*/
    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket!");
    // fill in server address structure
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[3]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    // bind socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding!");
    // listen
    listen(sockfd, 5);
	// accept only one request
	network_layersockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (network_layersockfd < 0)
			error("ERROR on accept");
		else
			printf("create new socket: %d\n", network_layersockfd);

	/*the main function will receive packets from the network layer and pass frames to wire*/
	// reset seq_num_count
	seq_num_count = 0;
	// communication loop
	while (1)
	{
		/*add codes to receive a packet from the network layer*/
		// empty buffer
		bzero(buffer,266);
		bzero(bufferPacket.nickname,10);
		bzero(bufferPacket.message,256);
		bzero(bufferFrame.my_packet.nickname,10);
		bzero(bufferFrame.my_packet.message,256);
		bzero(bufferFrame_str,269);
		// read from network layer
		ret = read(network_layersockfd, buffer, 265);
		if (ret < 0) 
            error("ERROR reading from socket!");
		else if (ret == 0) //indicate that client exit connection
            break;
		// convert buffer to packet
		bufferPacket = packetToRead(buffer);
		// display
		printf("Received a packet from network_layer\n");

		/* add codes to wrap the packet into a frame*/	
		// convert packet to frame
		bufferFrame = packet2frame(bufferPacket, seq_num_count, 0);
		// convert frame to string
		frameToSend(bufferFrame, bufferFrame_str);

		/*add codes to send the frame to the wire*/
		// write to wire
		ret = write(wiresockfd, bufferFrame_str, strlen(bufferFrame_str));
		if (ret < 0) 
            error("ERROR writing to socket!");
		else
		{
			printf("Sending a frame to wire ...\n");
			seq_num_count += 1;
		}

		/*if the message is "EXIT" */
		if (strcmp (bufferPacket.message, "EXIT\n")==0) 
		{
			pthread_cancel(wirepth); //kill the child thread
			close(wiresockfd);
			close (network_layersockfd); //close sockets
			return 0; //terminate the main function
		}

	}

}
