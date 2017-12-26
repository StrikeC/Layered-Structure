typedef struct packet_type {
	char nickname[10];
	char message[256];
}packet;

typedef struct frame_type {
	int seq_num;
	int type; // 1 for acknowledgment; 0 for data
	packet my_packet;
}frame;


