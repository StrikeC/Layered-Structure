
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


/*the thread function that receives frames from the wiresocket and sends packets to the network_layer */
void * rcvfromwiresend2network_layer ( char *argv[] )
{
	/*add codes to declare locals*/
	...

	 while (1)
	 {
		 /*add codes receive a frame from wire*/
		 ...

			 
		/*add codes to send the included packet to the network layer*/	
		...
	 }
}


int main(int argc, char *argv[])
{
	/*add codes to declear local variables*/
	...

	/*check numeber of arguments*/
     if (argc < 4) {
		 fprintf(stderr,"Usage: %s  wire__IP  wire_port data_port\n",argv[0] );
         exit(1);
     }

	/* add codes to connect to the wire. Assign value to gobal varialbe wiresockfd */
	...

	/*generate a new thread to receive frames from the wire and pass packets to the network layer */
	pthread_t wirepth;	// this is our thread identifier
	pthread_create(&wirepth,NULL,rcvfromwiresend2network_layer, NULL);

	/*add codes to create and listen to a socket that the network_layer will connect to. Assign value to global variable network_layersockfd*/
    ...

	/*the main function will receive packets from the network layer and pass frames to wire*/
	 while (1)
	 {
		/*add codes to receive a packet from the network layer*/
		...

		/* add codes to wrap the packet into a frame*/	
		...

		/*add codes to send the frame to the wire*/

		...

		/*if the message is "EXIT" */
		 if (strcmp (incoming_packet.message, "EXIT\n")==0) 
		 {
			 pthread_cancel(wirepth); //kill the child thread
			 close(wiresockfd);
			 close (network_layersockfd); //close sockets
			 return 0; //terminate the main function
		 }

	 }

}
