#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<time.h> 
#include<fcntl.h>

#include "window.h"
#include "UpperLayer.h"

/*********************************
 *	Purpose: A simulation of GBN and TCP protocols
 *********************************/

/*GLOBAL VARIABLES*/
int NextSeqNum = 0;
int SendBase = 0;
struct sockaddr_in servAddr;	//sockaddr for the server address
int sockFd;						//actual socket
int n,r;
struct Segment *segment;
int timer_duration;
time_t t1;
int time_out = 0;
int window_size;
int arrACK[1000];
int wait_for_timeout = 0;

/*********************************
 *	Function: 	make_packet
 *
 *	Purpose: 	Creates the actual segment
 *	
 *	Given: 		sequence_num which is the current sequence number of the created packet
 *				data which is a int32_t data that we got from the layer above
 *
 *	Returns:		struct Segment*, newly created segment
 *********************************/
struct Segment* make_packet(int sequence_num, int32_t data){
	unsigned char seg[12];
	struct Segment *segment = (struct Segment*)malloc(sizeof(struct Segment));
	bzero(segment, sizeof(struct Segment));
	
	segment->sequence = ntohl((uint32_t)sequence_num);
	segment->datalen = ntohl((uint32_t)4);
	segment->data = ntohl((int32_t)data);
	
	return segment;
}

/*********************************
 *	Function: 	send_packet
 *
 *	Purpose: 	sends the current segment to the recieving server
 *	
 *	Given: 		struct Segment* to send
 *
 *	Returns:	void
 *********************************/
void send_packet(struct Segment *segment){
	int length = sizeof(segment->sequence) + sizeof(segment->datalen) + sizeof(segment->data);
	n = sendto(sockFd, (char*)segment, length,0, (struct sockaddr*)&servAddr, sizeof(servAddr));
		
	//if n fails
	if (n < 0){
		perror("udpclient");
		fprintf(stdout,"Error occured during writing.\n");
		fprintf(stdout,"Segment#: %d", segment->sequence);
		fprintf(stdout,"Exiting.\n");
		exit(1);
	}else{
		//printf("\nRequest sent successfully");
	}
}

/*********************************
 *	Function: 	get_data
 *
 *	Purpose: 	recieves the reply (ACK) from the server on the socket
 *
 *	Returns:	0 if the data is not available
 *				1 if the data is available, through the global variable segment 
 *********************************/
int get_data(){
	int serv_len = sizeof(servAddr);
	unsigned char buffer[256];
	bzero(&buffer, sizeof(buffer));
	r = recvfrom(sockFd, (char*)buffer, 256 , 0, (struct sockaddr*)&servAddr, &serv_len);
	
	if (r < 0){
		return 0;
	}
	segment = (struct Segment*)&buffer[0];
	return 1;
}

/*********************************
 *	Function: 	get_event
 *
 *	Purpose: 	checks for the following, a timout event (either 3 duplicate acks or a timer timeout)
 *				if data is available from above
 *				if an ack is recieved from the server
 *
 *	Returns:	1 if data is available on the upper layer
 *				2 if a 3 duplicate acks are recieved, or a timeout
 *				3 if data is available on the socket 
 *********************************/
int get_event(){
	if(wait_for_timeout ==3){	//3 duplicate ACKs, or if window is full then force to resend because out of sync, therefore a packet was lost
		wait_for_timeout = 0;
		time_out = 0;
		return 2;
	}else{
		if(get_data()){			//check to see if data is available to be recieved back
			return 3;
		}
		if(time_out == 1){		//this creates a race condition between timout and recieving of data, maybe try to change the order around?
			time_out = 0;
			return 2;
		}
		if(isDataAvailable()){	//check to see if data is available from the layer above
			return 1;
		}
	}
	return -1;
}

/*********************************
 *	Function: 	print_time
 *
 *	Purpose: 	prints the current time event
 *
 *	Given:		int message, the number of the message
 *				int segment, the current segment 
 *
 *	Returns:	void 
 *********************************/
void print_time(int message, int segment){
	t1 = time(NULL);
	//printf("\n");
	switch(message){
		case 1:
			printf("Data retreived at %s", ctime(&t1));
			break;
		case 2:
			printf("Data (seq # %d) sent at %s", segment, ctime(&t1));
			break;
		case 3:
			printf("Ack # %d received at %s", segment, ctime(&t1));
			break;
		case 4:
			printf("Retransmit timer started at %s", ctime(&t1));
			break;
		case 5:
			printf("Retransmitting seq # %d at %s", segment, ctime(&t1));
			break;
	}
	printf("\n");
}

/*********************************
 *	Function: 	main_loop
 *
 *	Purpose: 	as the function name implies, it continuously polls get_event and depending on the return value does the following
 *				case1: gets the data from above, checks to see if current window_size is equal to window size, and either creates a new packet
 *				and sends it or sets wait_for_timeout so on the next iteration it resends the lowest unacked packet
 *				case2: resend the smallest unacked packet, after retriving the segment from the window
 *				case3: retrieves the data from the server socket, checks if its acked, if it is duplicate_ack++, else just removes it from the window
 *
 *	Given:		void 
 *
 *	Returns:	void 
 *********************************/
void main_loop(){
	int i, ACK, size;
	struct Segment* to_resend;
	for(;;){
		switch(get_event()){						//get the actual event
			case 1:
				size = get_size();					//get size
				if(size == window_size){			//if the size is equal to the window size, that means we wait for a packet to get acked or a timeout
					wait_for_timeout = 3;
					sleep(timer_duration);			//sleep for timer_duration (not like anything else we can do)
				}else{
					to_resend = get_from_list(NextSeqNum);
					if(to_resend != NULL){									//preserves data integretity by checking if it was already recieved from above
						print_time(5,NextSeqNum);
						send_packet(to_resend);
						alarm(timer_duration);
						print_time(4,0);
					}else{									//if its NOT already in the window, get the data from above
						print_time(1, 0);
						int32_t data = retreiveData();							//retrieve data from above
						struct Segment* seg = make_packet(NextSeqNum, data);	//create a segment
						printf("Seq:%d\nData length:%d\nData:%d\n", ntohl(seg->sequence), ntohl(seg->datalen), ntohl(seg->data));
						alarm(timer_duration);									//start timer 1
						add(seg);												//add the current segment into the linked list
						send_packet(seg);										//send the packet through the unreliable data transfer (UDP)
						print_time(2, ntohl(seg->sequence));
						print_time(4,0);
					}
					NextSeqNum++;
				}
				break;
			case 2:
				//SendBase is always the lowest ACKed packet, therefore SendBase+1 is the lowest UNACKed packet
				to_resend = get_from_list(SendBase+1);		//checks the LL for the segment with the appropiate sequence number
				if(to_resend != NULL){
					print_time(5, (SendBase+1));
					send_packet(to_resend);									//resend
					alarm(timer_duration);									//start timer 2 
					print_time(4,0);
					//wait_for_timeout++;
					NextSeqNum = SendBase+2;								//sequence number is SendBase+2
				}else{
					printf("\nPacket with sequence# %d was NOT FOUND\n", (SendBase+1));	
				}
				break;
			case 3:
				ACK = ntohl(segment->sequence);
				if(arrACK[ACK] == 0){										//if the recieved segment is unacked
					print_time(3, ACK);			
					arrACK[ACK] = 1;										//ack it
					//remove_element(ACK);
					for(i = SendBase; i<ACK; i++){							//sequence is cumulative
						arrACK[i] = 1;
						remove_element(ACK);								//ack recieved, remove the corresponding sequence number from the list
					}
					SendBase = ACK;				
				}else{
					//printf("\nDuplicate ACK %d!\n", (wait_for_timeout+1));
					wait_for_timeout++;
				}
				if(get_size() != 0){											//if there are still elements in the list, start time
					alarm(timer_duration);										//start timer 3
					print_time(4,0);
				}
				break;
			case -1:
				//printf("Skipping a cycle");
				break;
		}
	}
}

/*deals with a timeout event, sets time_out equal to 1 and calls main_loop*/
void timeout(){
	time_out = 1;
	main_loop();
}

/*********************************
 *	Function: 	init
 *
 *	Purpose: 	initalizes all the globals, creates a socket, and sends appropiate values to the LL and the upper layer 
 *
 *	Given:		char *argv[] which contains all the essential values
 *
 *	Returns:	void 
 *********************************/
void init(char *argv[]){
	int rec_port = atoi(argv[2]);
	window_size = atoi(argv[3]);
	timer_duration = atoi(argv[4]);
	int frequency = atoi(argv[5]);
	memset(arrACK, 0, sizeof(arrACK));			//clear the array to zeros
	signal(SIGALRM,timeout); 					//do this once
	
	printf("Basic Info:\nReciever address:%s	Reciever port:%d Window Size:%d Timeout:%d Frequency:%d\n\n", argv[1], rec_port, window_size, timer_duration, frequency);
	
	upperLayerInit(frequency);			//initialize the upper layer to send data every frequency amount of time
	set_window_size(window_size);		//initialize the linked list to maximum size of window_size

	if ((sockFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{
		perror("Error in socket():");
		exit(EXIT_FAILURE);
	}else{
		//printf("\nSocket Created");
	}

	int flags;
	if((flags=fcntl(sockFd,F_GETFL,0))<0){
		perror("Error in fcntl():");
		exit(EXIT_FAILURE);
	}
	
	flags = flags | O_NONBLOCK;
	
	if(fcntl(sockFd,F_SETFL,flags)<0){
		perror("Error in fcntl():");
		exit(EXIT_FAILURE);
	}
	
	//fill in remote server's info
	bzero(&servAddr, sizeof(servAddr));		//clear struct
	servAddr.sin_family = AF_INET;			//specify family
	servAddr.sin_port = htons(rec_port);		//specify port
	inet_pton(AF_INET, argv[1], &servAddr.sin_addr);  
}

/*main, checks for number of args, and then calls init() and main_loop()*/
int main(int argc, char *argv[]){
	//test();
	if(argc != 6){
		printf("Not Enough Parameters\n USAGE: ./tcp_sim <IP_reciever> <port_reciever> <window_size> <timer_duation(s)> <freq_of_data_gen(s)>");
		exit(0);
	}
	
	init(argv);
	main_loop();
}