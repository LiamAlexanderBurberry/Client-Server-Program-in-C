
//AUHTOR: LIAM BURBERRY GAHM, MAX STRANG
//Client.C file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "client.h"
#include <stdbool.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>




#define SEND_SYN 1
#define WAIT_SYN_ACK 2
#define SEND_ACK_SYN_ACK 3
#define GOT_SYN_ACK 4
#define GOT_CORRUPT 5
#define TIME_LIMIT 1
#define TIMEOUT 2
#define SET_WINDOW_SIZE 6
#define SEND 7
#define WAIT_ALL_ACK 8
#define RESEND_ALL 9
#define RECV1 10
#define RECV1_SLIDE 11
#define RECV2_SLIDE 12
#define RECV3_SLIDE 13
#define START_FIN 14
#define WAIT_FIN 15
#define REWAIT_ACKS 16
#define REWAITING 17


#define WAIT_FIN_ACK 18
#define SEND_FIN_ACK 19
#define GOT_FIN_ACK 20
#define WAIT_FIN 21
#define GOT_FIN_ACKACK 22
#define GOT_FIN 23
#define SLIDE_SEND 24
#define SLIDE_WAIT 25
#define SLIDE_REWAIT_ACKS 26
#define CONTINUE_REWAITING 27






uint16_t calculateChecksum(const char *buff, size_t length)
{
    uint32_t sum = 0; //sum of all 16bit values in the packet buffer
	
    //looping through the buffer
	while(length > 1)
	{
		sum += (uint16_t)(*buff);//casting the value of buff to a 16bit value and adding it to the sum
		buff++;
		length -= 2;
	}
	if(length != 0)
	{
		sum += (uint16_t)*buff; //if the length is not even we have to add the last byte to the sum
	}
	while(sum >> 16) //checks if the value of sum is equal to 2^16, if it is we have to fold the extra bits. 
	{
		sum = (sum & 0xFFF) + (sum >> 16); //folding operation if needed.
	}
	return (uint16_t)~sum; //returnning the first complement of the calculated bits. This is the checksum!
}


void initFinAckAck(custom_header *header) //initializes a fin ack ack packet
{
    //Uses a pointer to the packet called header to initialize the packet.
    header->flag = FLAG_FIN_ACKACK;
    header->seqNum = 3;
    header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));
}

void initFinAck(custom_header *header) //initializes a fin ack packet
{
    //Uses a pointer to the packet called header to initialize the packet.
    header->flag = FLAG_FIN_ACK;
    header->seqNum = 2;
    header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));
}

void initFin(custom_header *header) //initializes a fin packet
{
    //Uses a pointer to the packet called header to initialize the packet.
    header->flag = FLAG_FIN;
    header->seqNum = 1;
    header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));
}


void initSlidingPackets(custom_header *header, int size, int socket_desc, char client_message[2000], char server_message[2000], socklen_t length, struct sockaddr_in server_addr, int server_struct_length)
{
    int j = 1;
    int x = 0;
    //THIS FUNCTION INITIALIZES THE SLIDING WINDOW PACKETS AND SENDS THEM ONE BY ONE WITH A 2 SECOND PAUSE AFTER EACH INDIVIDUAL SENDING. 
    for(int i = 1; i < header->windowSize+1; i++)
    {
        header->data = i;
        header->seqNum = i;
        header->flag = FLAG_PACKET;
        header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));//CALCULATES THE CHECKSUM OF THE PACKET
        copyHeadToBuff(header, client_message);//COPIES PACKET AND HEADER INFO TO THE BUFFER
        sendTo(socket_desc, client_message, sizeof(client_message), server_addr, server_struct_length);
        clear(server_message, client_message, header);
        setWindowSize(header);//SETS THE WINDOW SIZE TO 3
        printf("Packet %d sent successfully!\n", j);
        j++;
        sleep(2);
    }
    
}

void setWindowSize(custom_header *header)//FUNCTION TO SET WINDOW SIZE, USES A POINTER TO THE PACKET TO INITIALIZE THE WINDOW SIZE AND WINDOW CORRECTION
{
    header->windowSize = 3;
    header->windowCorrection = 1;
}

void initSynAckAck(custom_header *header)//FUNCTION TO INITIALIZE A SYN ACK ACK PACKET
{
    header->flag = FLAG_SYN_ACKACK;
    header->seqNum = 3;
    header->unCalculatedCheckSum = 0;
    header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));
}

void initSyn(custom_header *header)//FUNCTION TO INITIALIZE A SYN PACKET
{
    header->flag = FLAG_SYN;
    header->seqNum = 1;
    header->unCalculatedCheckSum = 0;
    header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));
    

}

void clear(char server_message[2000], char client_message[2000], custom_header *header) //FUNCTION TO CLEAR THE BUFFERS AND THE PACKET/HEADER
{
    memset(server_message, 0, sizeof(server_message));
    memset(client_message, 0, sizeof(client_message));
    memset(header, 0, sizeof(header));
}

void copyHeadToBuff(custom_header *header, char client_message[2000])//FUNCTION TO COPY THE HEADER TO THE BUFFER
{
    memcpy(client_message, header, sizeof(custom_header));
}

void copyBuffToHead(custom_header *header, char client_message[2000]) //FUNCTION TO COPY THE BUFFER TO THE HEADER
{
    memcpy(header, client_message, sizeof(client_message));
}

int definePacket(custom_header *header, char client_message[2000]) //DEFINE PACKET FUNCTION USED TO DEFINE WHAT KIND OF PACKET/ACK THE CLIENT HAS RECEIVED. 
{
    int cSum = header->checkSum;
    header->checkSum = 0;

    

    if(cSum == calculateChecksum((char*)header, sizeof(custom_header)))//CHECKS IF THE CHECKSUM IS VALID/THE EXEPECTED ONE. 
    {
        if(header->flag == FLAG_SYN_ACK)//IF THE FLAG IS SYN ACK, THE CLIENT HAS RECEIVED A SYN ACK PACKET
        {
            return GOT_SYN_ACK;
        }

        if(header->flag == FLAG_FIN_ACK && header->seqNum == 2) //IF THE FLAG IS FIN ACK AND THE SEQNUM IS 2, THE CLIENT HAS RECEIVED A FIN ACK PACKET
        {
            return GOT_FIN_ACK;
        }
        if(header->flag == FLAG_FIN && header->seqNum == 1) //IF THE FLAG IS FIN AND THE SEQNUM IS 1, THE CLIENT HAS RECEIVED A FIN PACKET
        {
            return GOT_FIN;
        }
        if(header->flag == FLAG_FIN_ACKACK && header->seqNum == 3) //IF THE FLAG IS FIN ACK ACK AND THE SEQNUM IS 3, THE CLIENT HAS RECEIVED A FIN ACK ACK PACKET
        {
            return GOT_FIN_ACKACK;
        }
    }
}

int slidingDefine(custom_header *header, int arr[3])//SAME FUNCTION AS THE DEFINE PACKET FUNCTION BUT THIS IS MAINLY USED DURING ACTION OF THE SLIDING WINDOW PROTOCOL.
{
    int event = 0;
    
    int cSum = 0;
    cSum = header->checkSum;
    header->checkSum = 0;
    
    if(cSum == calculateChecksum((char*)header, sizeof(custom_header)))//CHECKS CHECKSUM
    {
        
        if(header->flag == FLAG_ACK && header->ackNum == 1) //FIRST ACK HAS BEEN RECEIVED AFTER SENDING OF PACKET 1,2,3 TO THE SERVER.
        {
            if(arr[0]==1)
            {
                printf("\nFirst Ack was received again!\n");//FIRST ACK RECEIVED AGAIN, THIS IS THE ACK FOR PACKET 1
                event = RECV1_SLIDE;
                return event;

            }
            else if(arr[0] == 0 && arr[1] == 0 && arr[2] == 0)//FIRST PACKET RECEIVED IN ORDER
            {
                
                arr[0] = 1; 
                printf("\nFirst Ack was received in order!\n");
                event = RECV1_SLIDE;
                return event;
                //slide 1
                

            }
            else if(arr[1] == 2 && arr[2] == 3)//FIRST PACKET RECEIVED AFTER PACKET 2,3 HAS BEEN RECEIVED
            {
                arr[0] = 1;
                //slide 1, 2, 3
                event = RECV1_SLIDE;
                return event;
               
            }
            else if(arr[1] == 2)//FIRST PACKET RECEIVED AFTER PACKET 2 HAS BEEN RECEIVED
            {
                arr[0] = 1;
                event = RECV1_SLIDE;
                return event;
                //slide 1, 2
            }
            
        }
        if(header->flag == FLAG_ACK && header->ackNum == 2)//SAME BUT FOR THE SECOND PACKET
        {
            //HÄR MOTTAS ANDRA
            if(arr[1] == 2)
            {
                printf("Second Ack was received again!\n");
                event = RECV2_SLIDE;
                return event;
            }
            if(arr[0] == 1 && arr[2] == 3)
            {
                arr[1] = 2;
                event = RECV2_SLIDE;
                return event;
                //slide 2, 3
            }
            else if(arr[0] == 1)
            {
                printf("\nSecond Ack was received in order!\n");
                arr[1] = 2;
                event = RECV2_SLIDE;
                return event;
                //slide 2
            }
            else
            {
                //saving 2 in buffer
                arr[1] = 2;
                arr[1] = 2;
                event = RECV2_SLIDE;
                return event;
                
                //No slide
                
            }
            
            
        }
        if(header->flag == FLAG_ACK && header->ackNum == 3)//SAME BUT FOR THE THIRD PACKET
        {
            
            //HÄR MOTTAS TREDJE
            if(arr[2] == 3)
            {
                printf("Third Ack was received again!\n");
                event = RECV3_SLIDE;
                return event;
                //resending ack of packet 3.
            }
            if(arr[0] == 1 && arr[1] == 2 )
            {
                printf("\nThird Ack was received in order!\n");
                //exit(EXIT_SUCCESS);
                arr[2] = 3;
                event = RECV3_SLIDE;
                return event;
                //slide 3
            }
            else if(arr[1] == 2)//Betyder att A1 ej finns
            {
                arr[2] = 3; //väntar på A1
                //No slide
                event = RECV3_SLIDE;
                return event;
            }
            else if (arr[0] == 1) //Betyder att A2 ej finns
            {
                arr[2] = 3; //väntar på A2
                //No slide
                event = RECV3_SLIDE;
                return event;
            }
            else
            {
                //BETYDER ATT INGEN FINNS
                arr[2] = 3;
                event = RECV3_SLIDE;
                return event;
            }
            
        }
    }

    //HERE A CORRUPT ACK WAS RECEIVED -> FULL RETRANSMISSION OF ACKS
    else
    {
       
        return GOT_CORRUPT;

    }
    
}

int readable_timeo(int fd, int sec)//THIS IS OUR TIMER FUNCTION AND IS A FUCTION EXISTING IN C THAT CAN BE USED TO STOP THE PROGRAM WHILE RECEVING SOMETHING FOR A SPECIFIED AMOUNT OF TIME.
{
    fd_set rset; //FILE DESCRIPTOR SET
    struct timeval tv; //TIMEVAL STRUCTURE == TIME VALUE

    FD_ZERO(&rset); //INITIALIZES THE FILE DESCRIPTOR SET TO HAVE ZERO BITS FOR ALL FILE DESCRIPTORS
    FD_SET(fd, &rset); //SETS THE BIT FOR THE FILE DESCRIPTOR fd IN THE FILE DESCRIPTOR SET rset


    tv.tv_sec = sec; //SETS THE TIME TO WAIT FOR THE FILE DESCRIPTOR TO BE READY
    tv.tv_usec = 0; //SETS THE TIME TO WAIT FOR THE FILE DESCRIPTOR TO BE READY
    return(select(fd + 1, &rset, NULL, NULL, &tv)); //SELECT() BLOCKS THE CALLER UNTIL THE FILE DESCRIPTOR IS READY OR THE TIME HAS ELAPSED
}

int timer(custom_header *header, int socket_desc, char server_message[2000], socklen_t length, struct sockaddr_in server_addr, int server_struct_length)//THIS FUNCTION IS THE TIMER FUNCTION THAT USES THE READABLE_TIMEO FUNCTION TO WAIT FOR A SPECIFIED AMOUNT OF TIME BEFORE RETURNING A VALUE.
{
    int event = 0;
    int count = readable_timeo(socket_desc, 2);
    if(count > 0)
    {
        if(recvfrom(socket_desc, server_message, sizeof(server_message), 0,
            (struct sockaddr*)&server_addr, &server_struct_length) < 0){
            printf("Couldn't receive anything, UNKNOWN ERROR...\n");
            exit(EXIT_FAILURE);
        }
        copyBuffToHead(header, server_message);
        event = definePacket(header, server_message);//CHECKS WHAT KIND OF PACKET HAS BEEN RECEIVED.
        return event;
    }
    else if(count == 0)
    {
        //Resending...
        return event;
    }               
    
     
    while(1)
    {
        
        if(recvfrom(socket_desc, server_message, sizeof(server_message), 0,
            (struct sockaddr*)&server_addr, &server_struct_length) < 0){
            printf("Couldn't receive anything, UNKNOWN ERROR...\n");
            exit(EXIT_FAILURE);
        }
        copyBuffToHead(header, server_message);
        event = definePacket(header, server_message);
        return event;
        
    }
    return event;
}


int slidingTimer(custom_header *header, int socket_desc, char server_message[2000], socklen_t length, struct sockaddr_in server_addr, int server_struct_length, int arr[3], int recvCount)//SAME AS TIMER BUT FOR SLIDING WINDOW, ALSO USES THE READABLE_TIMEO FUNCTION.
{
    int event = 0;
    int count = readable_timeo(socket_desc, 4);//START TIMER
    
    
    if(count > 0)
    {
        if (recvfrom(socket_desc, server_message, sizeof(server_message), 0,
        (struct sockaddr*)&server_addr, &server_struct_length) < 0){
        printf("Couldn't receive\n");
        return -1;
        }
    
        recvCount++;
        memcpy(header, server_message, sizeof(server_message));
        printf("%d\n",header->ackNum);
        event = slidingDefine(header, arr);
        return event;//SOMETHING WAS RECEIVED
    }
    if(recvCount < 3)
    {
        if(arr[0] == 0 && arr[1] == 0 && arr[2] == 0)
        {
            printf("Something went wrong...\n");//SOMETHING WENT WRONG==NO ACKS RECEIVED
            event = RESEND_ALL;
            return event;
        }
        if(recvCount != 3)
        {
            printf("Something is wrong with the acks!\n");//A FEW BUT NOT ALL ACKS WERE RECEIVED
            event = CONTINUE_REWAITING;
            return event;
        }
    }
    
    
    
}


void sendTo(int socket_desc, char client_message[2000], socklen_t length, struct sockaddr_in server_addr, int server_struct_length)
{
    int rnd = rand() % 10 + 1;
    if(rnd >= 8)
    {
        if(sendto(socket_desc, client_message, sizeof(client_message), 0,
        (struct sockaddr*)&server_addr, server_struct_length) < 0){
        printf("Can't send, UNKNOWN ERROR...\n");
        printf("Exiting program...\n");
        exit(EXIT_FAILURE);
        }
    }
    
   
    
}

int main(void){
    int socket_desc;//SOCKET DESCRIPTOR
    int event = 0; //EVENT VARIABLE USED FOR STATE MACHINES
    int state = SEND_SYN;//STARTING STATE FOR CLIENT
    bool setup = false;//BOOLEAN VARIABLE USED TO CHECK IF THE STATE MACHINES ARE COMPLETE
    bool procedure = false;//-||-
    bool termination = false;//-||-
    struct sockaddr_in server_addr; //SOCKET ADDRESS STRUCTURE
    char server_message[2000], client_message[2000]; //STRINGS FOR SENDING AND RECEIVING MESSAGES == "BUFFERS"
    int server_struct_length = sizeof(server_addr); //LENGTH OF THE SERVER ADDRESS STRUCTURE
    custom_header *header = (custom_header*)malloc(sizeof(custom_header)); //HEADER STRUCTURE -> ALLOCATING MEMORY
    srand(time(NULL)); //RANDOMIZE FUNCTION FOR SIMULATING PACKET LOSS. 
    

    memcpy(client_message, header, sizeof(custom_header));
    
    // Create socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    
    
    while(setup == false)
    {
        switch (state)
        {
            case SEND_SYN:
            
                clear(server_message, client_message, header); //CLEARS THE BUFFERS AND HEADER
                initSyn(header); //INITIALIZES THE HEADER FOR SYN PACKET
                copyHeadToBuff(header, client_message); //COPIES THE HEADER TO THE BUFFER
                sendTo(socket_desc, client_message, sizeof(client_message), server_addr, server_struct_length); //SENDS THE BUFFER TO THE SERVER

                
                clear(server_message, client_message, header);
                event = timer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length);
                if(event == 0) //IF NOTHING IS RECEIVED
                {
                    printf("Syn was lost... RESENDING!\n");
                    sleep(4); //TIMEOUT THEN RESENDING OF SYN
                    state = SEND_SYN;
                }
                else
                {
                    state = WAIT_SYN_ACK; //IF SOMETHING IS RECEIVED, GO TO THE NEXT STATE
                }
                
                break;

            case WAIT_SYN_ACK:
                if(event == GOT_SYN_ACK) //IF SYNACK WAS RECEIVED
                {
                    printf("SYNACK received\n");
                    
                    printf("Sending final acknowledgement...\n");
                    //sleep(1);
                    clear(server_message, client_message, header);
                    initSynAckAck(header);
                    copyHeadToBuff(header, client_message);
                    sendTo(socket_desc, client_message, sizeof(client_message), server_addr, server_struct_length);
                    sleep(2);
                    printf("Connection established!\n");
                    setup = true;
                    //exit(EXIT_SUCCESS);
                    
                }
                else if(event == GOT_CORRUPT) //IF CORRUPT PACKET WAS RECEIVED
                {
                    printf("Corrupt packet received...\n");
                    exit(EXIT_FAILURE);
                }
                else if(event == 0) //ERROR IN THE PROGRAM IF FUNCTION RETURNS 0 AGAIN. 
                {
                    printf("Timeout\n");
                    exit(EXIT_FAILURE);
                }
                else//IF ANYTHING ELSE IS RECEIVED UNKNOWN ERROR
                {
                    printf("Unknown event\n");
                    exit(EXIT_FAILURE);
                }
            
        }
    }

    procedure = false; //BOOLEAN VARIABLE USED TO CHECK IF THE STATE MACHINES ARE COMPLETE
    event = 0;
    state = SET_WINDOW_SIZE;
    int arr[3] = {0};
    int arr2[3] = {0};
    int recvCount = 0;
    int count = 0;
    //CLIENT SLIDING WINDOW PROTOCOL:
    while(procedure == false)
    {
        switch (state)
        {
            case SET_WINDOW_SIZE : 
                printf("\nSetting window size...\n");
                sleep(1);
                clear(server_message, client_message, header);
                setWindowSize(header);
                state = SEND;
            break;

            case SEND : 

                initSlidingPackets(header, header->windowSize, socket_desc, client_message, server_message, sizeof(client_message), server_addr, server_struct_length);
                //WAIT STATE!
                state = WAIT_ALL_ACK;
                clear(server_message, client_message, header);
                event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
            break;

            case WAIT_ALL_ACK : //WAITING FOR ALL ACKS TO BE RECEIVED
                if(event == CONTINUE_REWAITING) //IF NOT ALL HAS BEEN RECV WE CONTINUE TO REWAIT. 
                {
                    event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                }
                if(event == RESEND_ALL)//IF NOTHING WAS RECEIVED WE RESEND IT ALL
                {
                    printf("\nNothing was received, resending all...\n");
                    //sleep(2);
                    clear(server_message, client_message, header);
                    int j = 1;
                    
                    state = SEND;
                    
                    
                }

                if(event == RECV1_SLIDE)//FIRST ACK RECV
                {
                    arr[0] = 1;
                    printf("First ack recv\n");
                    clear(server_message, client_message, header);
                    if(arr[0] == 1 && arr[1] == 2 && arr[2] == 3)//CHECKS IF ALL ACKS HAVE BEEN RECV
                    {
                        printf("All acks have been received\n");
                        
                        state = SLIDE_SEND;
                        //printf("Moving on to termination of program...\n");
                        //procedure = true;
                    }
                    else//KEEP ON WAITING
                    {
                        event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                        if(event == 0)
                        {
                            state = REWAIT_ACKS;
                            event = REWAITING;
                        }
                    }
                    
                }

                if(event == RECV2_SLIDE)//SAME HERE BUT FOR PACKET 2
                {
                    arr[1] = 2;
                    printf("Second ack recv\n");
                    if(arr[0] == 1 && arr[1] == 2 && arr[2] == 3)
                    {
                        printf("All acks have been received\n");
                        state = SLIDE_SEND;
                        //printf("Moving on to termination of program...\n");
                        //procedure = true;
                    }
                    else
                    {
                        clear(server_message, client_message, header);
                        event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                        if(event == 0)
                        {
                            state = REWAIT_ACKS;
                            event = REWAITING;
                        }
                    }
                }
                
                if(event == RECV3_SLIDE)//SAME HERE BUT FOR PACKET 3
                {
                    arr[2] = 3;
                    
                    printf("Third ack recv\n");
                    
                    if(arr[0] == 1 && arr[1] == 2 && arr[2] == 3)
                    {
                        printf("\n\nAll acks have been received\n");
                        //sleep();
                        state = SLIDE_SEND;
                        //printf("Moving on to termination of program...\n");
                        //procedure = true;
                    }
                    else
                    {
                        clear(server_message, client_message, header);
                        event = 0;
                        event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                        if(event == 0)
                        {
                            state = REWAIT_ACKS;
                            event = REWAITING;
                        }
                        
                    }

                    //exit(EXIT_SUCCESS);
                }
                
            break;

            case REWAIT_ACKS ://REWAITING OF ACKS SINCE SOME WERE LOST OR CORRUPT
                if(event == REWAITING)
                {
                    printf("\nAcks received were either lost or corrupt!\n");
                    printf("Demanding a full retransmission...\n");
                    sleep(1);
                    clear(server_message, client_message, header);
                    header->ackNum = 16;
                    header->flag = NAK;
                    header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));
                    copyHeadToBuff(header, client_message);
                    //sendTo(socket_desc, client_message, sizeof(client_message), server_addr, server_struct_length);
                    clear(server_message, client_message, header);
                    state = WAIT_ALL_ACK;
                    event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                    //event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                    
                }
            break;

            case SLIDE_SEND : //SLIDE THE FIRST SET OF PACKETS AS THE EXPECTED ACKS HAVE BEEN RECEIVED
                printf("is here!!!!\n");
                sleep(3);
                memset(arr, 0, sizeof(arr));
                memset(arr2, 0, sizeof(arr2));
                clear(server_message, client_message, header);
                initSlidingPackets(header, header->windowSize, socket_desc, client_message, server_message, sizeof(client_message), server_addr, server_struct_length);
                //WAIT STATE!
                state = SLIDE_WAIT;
                clear(server_message, client_message, header);
                event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
            break;



            case SLIDE_WAIT ://WAITING FOR THE SENCOND SET OF ACKS
                 
                if(event == CONTINUE_REWAITING)//CONTINUE WAITING WHEN RETRANSMISSION HAPPENED
                {
                    event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                }

                if(event == RESEND_ALL)//IF NOTHING WAS RECV WE RESEND ALL
                {
                    printf("\nNothing was received, resending all...\n");
                    //sleep(2);
                    clear(server_message, client_message, header);
                    int j = 1;
                    
                    
                    
                    //FOR SIMULATING PACKET LOSS OF THE THIRD PACKET
                    for(int i = 1; i < header->windowSize+1; i++)
                    {
                        header->data = i;
                        header->seqNum = i;
                        header->flag = FLAG_PACKET;
                        header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));
                        copyHeadToBuff(header, client_message);
                        sendTo(socket_desc, client_message, sizeof(client_message), server_addr, server_struct_length);
                        clear(server_message, client_message, header);
                        setWindowSize(header);
                        printf("Packet %d sent successfully!\n", j);
                        j++;
                        sleep(2);
                    }
                    
                    clear(server_message, client_message, header);
                    printf("Moving to receiving state...\n");
                    arr[0] = 0;
                    arr[1] = 0;
                    arr[2] = 0;
                    event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                    
                    //state = SEND;
                }

                if(event == RECV1_SLIDE) //CHECKS WHAT ACK RECV
                {
                    arr[0] = 1;
                    printf("First ack recv\n");
                    if(arr[0] == 1 && arr[1] == 2 && arr[2] == 3)
                    {
                        printf("All acks have been received\n");
                        state = START_FIN;
                        printf("Moving on to termination of program...\n");
                        procedure = true;
                    }
                    else
                    {
                        event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                        if(event == 0)
                        {
                            state = SLIDE_REWAIT_ACKS;
                            event = REWAITING;
                        }
                    }
                    
                }

                if(event == RECV2_SLIDE) //CHECKS WHAT ACK RECV
                {
                    arr[1] = 2;
                    printf("Second ack recv\n");
                    if(arr[0] == 1 && arr[1] == 2 && arr[2] == 3)
                    {
                        printf("All acks have been received\n");
                        state = START_FIN;
                        printf("Moving on to termination of program...\n");
                        procedure = true;
                    }
                    else
                    {
                        event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                        if(event == 0)
                        {
                            state = SLIDE_REWAIT_ACKS;
                            event = REWAITING;
                        }
                    }
                }
                
                if(event == RECV3_SLIDE) //CHECKS WHAT ACK RECV
                {
                    arr[2] = 3;
                    
                    printf("Third ack recv\n");
                    if(arr[0] == 1 && arr[1] == 2 && arr[2] == 3)
                    {
                        printf("\n\nAll acks have been received\n");
                        state = START_FIN;
                        printf("Moving on to termination of program...\n");
                        procedure = true;
                    }
                    else
                    {
                        event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                        if(event == 0)
                        {
                            state = SLIDE_REWAIT_ACKS;
                            event = REWAITING;
                        }
                        
                    }

                    //exit(EXIT_SUCCESS);
                }
                
            break;

            case SLIDE_REWAIT_ACKS :
                if(event == REWAITING)
                {
                    printf("\nAcks received were either lost or corrupt!\n");
                    printf("Demanding a full retransmission...\n");
                    sleep(1);
                    clear(server_message, client_message, header);
                    header->ackNum = 16;
                    header->flag = NAK;
                    header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));
                    copyHeadToBuff(header, client_message);
                    //sendTo(socket_desc, client_message, sizeof(client_message), server_addr, server_struct_length);
                    clear(server_message, client_message, header);
                    state = WAIT_ALL_ACK;
                    event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                    //event = slidingTimer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length, arr2, recvCount);
                }
            break;

        }
    }

    
    

    //STATE MACHINE FOR CONNECTION TEARDOWN
    bool finish = false;
    int end = 1;
    
    
        printf("The client will issue a connection teardown request!\n");
        printf("Request pending...\n");
        state = START_FIN;
        sleep(4);
  
    



    while(finish == false && end == 1)
    {
        
        
        switch(state)
        {
             case START_FIN : //STARTING TEARDOWN REQUEST
                clear(server_message, client_message, header);
                initFin(header);
                memcpy(client_message, header, sizeof(custom_header));
                sendTo(socket_desc, client_message, sizeof(client_message), server_addr, server_struct_length);
                state = WAIT_FIN_ACK;
                //sleep(1);
                event = timer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length);
            break;

            int counter = 0;

            case WAIT_FIN_ACK : //WAITING FOR FIN ACK
                    clear(server_message, client_message, header);

                    if(event == GOT_FIN_ACK)
                    {
                        printf("Got fin ack. Preparing to send final ack!\n");
                        initFinAckAck(header);
                        memcpy(client_message, header, sizeof(custom_header));
                        sendTo(socket_desc, client_message, sizeof(client_message), server_addr, server_struct_length);
                        event = timer(header, socket_desc, server_message, sizeof(server_message), server_addr, server_struct_length);
                    
                        
                        printf("Assuming teardown complete!\n");//ASSUMING TEARDOWN COMPLETE EVEN IF FIN ACK IS LOST
                        printf("Exiting...\n");
                        finish = true;
                        
                    }
                    if(event == GOT_CORRUPT)//IF CORRUPT ACK IS RECEIVED -> RESEND FIN
                    {
                        printf("A corrupt ack has been received\n");
                        printf("resending FIN!\n");
                        state = START_FIN;
                    }
                    if(event == 0) //NOTHING RECEIVED -> RESEND FIN 
                    {
                        printf("Fin must have been lost on the way!\n");
                        printf("Resending!\n");
                        count++; //COUNTER TO KEEP TRACK OF TIME, IF TIMER EXPIRES 4 TIMES -> SERVER MUST BE DOWN -> EXIT
                        sleep(2);
                        state = START_FIN;
                        if(count == 4)
                        {
                            printf("Server must be down!\n");
                            printf("Exiting...\n");
                            exit(EXIT_SUCCESS);
                        }
                    }


        }
    }
    
    
exit(EXIT_SUCCESS);
    
    
   
    
    return 0;
}
