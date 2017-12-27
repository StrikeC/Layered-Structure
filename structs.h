
typedef struct packet_type {
	char nickname[10];
	char message[256];
}packet;

typedef struct frame_type {
	int seq_num;
	int type; // 1 for acknowledgement; 0 for data
	packet my_packet;
}frame;

// convert packet to string
void packetToSend(packet packetIn_p, char packetOut_s[266]) {
	// get header
	int len_nickname;
	len_nickname = strlen(packetIn_p.nickname);
	// generate string
	bzero(packetOut_s, 266);
	sprintf(packetOut_s, "%d%s%s", len_nickname, packetIn_p.nickname, packetIn_p.message);
}

// convert string to packet
packet packetToRead(char packetIn_s[266]) {
	packet packetOut_p;
	int len_nickname;
	// extract length of nickname
	len_nickname = packetIn_s[0] - '0';
	// extract packet
	bzero(packetOut_p.nickname, 10);
	bzero(packetOut_p.message, 256);
	strncpy(packetOut_p.nickname, packetIn_s+1, len_nickname);
	strncpy(packetOut_p.message, packetIn_s+(1+len_nickname),(strlen(packetIn_s)-1-len_nickname));
	return packetOut_p;
}

// convert frame to string
void frameToSend(frame frameIn_f, char frameOut_s[269]) {
	// get header
	int len_nickname;
	char seq_num_ch1, seq_num_ch2;
	len_nickname = strlen(frameIn_f.my_packet.nickname);
	seq_num_ch1 = (frameIn_f.seq_num/10) + '0';
	seq_num_ch2 = (frameIn_f.seq_num-((frameIn_f.seq_num/10)*10)) + '0';
	// generate string
	bzero(frameOut_s, 269);
	sprintf(frameOut_s, "%d%c%c%d%s%s", len_nickname, seq_num_ch1, seq_num_ch2, frameIn_f.type, frameIn_f.my_packet.nickname, frameIn_f.my_packet.message);
}

// convert string to frame
frame frameToRead(char frameIn_s[269]) {
	frame frameOut_f;
	int len_nickname;
	// extract length of nickname
	len_nickname = frameIn_s[0] - '0';
	// extract frame
	bzero(frameOut_f.my_packet.nickname, 10);
	bzero(frameOut_f.my_packet.message, 256);
	frameOut_f.seq_num = (frameIn_s[1] - '0')*10 + (frameIn_s[2] - '0');
	frameOut_f.type = frameIn_s[3] - '0';
	strncpy(frameOut_f.my_packet.nickname, frameIn_s+4, len_nickname);
	strncpy(frameOut_f.my_packet.message, frameIn_s+(4+len_nickname),(strlen(frameIn_s)-4-len_nickname));
	return frameOut_f;

}

// convert packet to frame
frame packet2frame(packet packetIn, int seq_num, int type) {
	frame frameOut;
	frameOut.seq_num = seq_num;
	frameOut.type = type;
	strcpy(frameOut.my_packet.nickname, packetIn.nickname);
	strcpy(frameOut.my_packet.message, packetIn.message);
	return frameOut;
}

// convert frame to packet
packet frame2packet(frame frameIn) {
	packet packetOut;
	strcpy(packetOut.nickname, frameIn.my_packet.nickname);
	strcpy(packetOut.message, frameIn.my_packet.message);
	return packetOut;
}
