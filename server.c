//AUHTOR: LIAM BURBERRY GAHM, MAX STRANG
//Server.C file.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "server.h"



//THESE ARE SOME HELP VARIABLES FOR THE PROGRAM

#define WAIT_SYN 1
#define SEND_SYN_ACK 2
#define WAIT_ACK_SYN_ACK 3
#define GOT_SYN 4
#define GOT_ACK 5
#define GOT_CORRUPT 6
#define TIME_LIMIT 1
#define TIMEOUT 2
#define SET_WINDOW_SIZE 7
#define RECV1 8
#define RECV2 9
#define RECV3 10
#define SEQ_VALID_SAVE_PACK1 11
#define SEQ_VALID_SAVE_PACK2 12
#define SEQ_VALID_SLIDE 13
#define RECV1_SLIDE 14
#define RECV2_SLIDE 15
#define RECV2_SAVE 16
#define RECV3_SLIDE 17
#define RECV3_SAVE 18
#define SEND_ALL_ACKS 19
#define NAK 20

#define WAIT_FIN 21
#define SEND_FIN_ACK 22
#define GOT_FIN 23
#define GOT_FIN_ACKACK 23
#define GOT_FIN_ACK 24
#define CHECK 25
#define SLIDE 26
#define SLIDE_SEND_ACKS 27

uint16_t calculateChecksum(const char *buff, size_t length)//THIS IS THE FUNCTION FOR CALCULATING THE CHECKSUM
{
    uint32_t sum = 0; //sum of all 16bit values in the packet buffer
	
	while(length > 1)
	{
		sum += (uint16_t)(*buff);//CASTING THE BUFFER TO 16BIT SO THAT WE CAN ITERATE OVER IT IN THE LOOP
		buff++;
		length -= 2;
	}
	if(length != 0)
	{
		sum += (uint16_t)*buff;//SUMS UP THE LAST BYTE IF THE LENGTH IS ODD
	}
	while(sum >> 16) //checks if the value of sum is equal to 2^16, if it is we have to fold the extra bits. 
	{
		sum = (sum & 0xFFF) + (sum >> 16); //folding operation if needed.
	}
	return (uint16_t)~sum; //returnning the first complement of the calculated bits. This is the checksum!
}

void initFinAck(custom_header *header)//THIS FUNCTION INITIALIZES THE FIN_ACK PACKET USING A STRUCT CALLED HEADER. IN OUR CASE THE HEADER POINTER IS A POINTER TO THE WHOLE PACKET
{
    header->flag = FLAG_FIN_ACK; //A ENUMERATED TYPE FLAG IS USED TO SET THE FLAG FIELD OF THE HEADER
    header->seqNum = 2; //THIS IS THE SEQUENCE NUMBER OF THE PACKET
    header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));//SENDS THE SIZEOF THE PACKET TO THE CALCULATE CHECKSUM FUNCTION
}

void initFin(custom_header *header)//THIS FUNCTION INITIALIZES THE FIN PACKET USING A STRUCT CALLED HEADER. IN OUR CASE THE HEADER POINTER IS A POINTER TO THE WHOLE PACKET
{
    header->flag = FLAG_FIN;//A ENUMERATED TYPE FLAG IS USED TO SET THE FLAG FIELD OF THE HEADER
    header->seqNum = 1; //THIS IS THE SEQUENCE NUMBER OF THE PACKET
    header->checkSum = calculateChecksum((char*)header, sizeof(custom_header)); //SENDS THE SIZEOF THE PACKET TO THE CALCULATE CHECKSUM FUNCTION
}

void initAck(custom_header *header, int ackNumber, int socket_desc, char server_message[2000], socklen_t server_len, struct sockaddr_in client_addr, int client_struct_length)//THIS FUNCTION INITIALIZES THE ACK PACKET FOR THE SLIDING WINDOW PROTOCOL
{
    
    for(int i = 0; i < 3; i++)//THE LOOP INITIATES THE PACKET AND SENDS IT AND DOES SO THREE TIMES, AS THE WINDOW SIZE IS 3
    {
        
        header->ackNum = ackNumber; //THIS IS THE ACKNOWLEDGEMENT NUMBER OF THE PACKET
        header->flag = FLAG_ACK; //A ENUMERATED TYPE FLAG IS USED TO SET THE FLAG FIELD OF THE HEADER
        header->checkSum = calculateChecksum((char*)header, sizeof(custom_header)); //SENDS THE SIZEOF THE PACKET TO THE CALCULATE CHECKSUM FUNCTION
        memcpy(server_message, header, sizeof(custom_header)); //COPIES THE HEADER TO THE SERVER MESSAGE BUFFER
        sendTo(socket_desc, server_message, client_addr, client_struct_length);
        clear(server_message, server_message, header);
        sleep(2);
        ackNumber++;
    }
   
}

void setWindowSize(custom_header *header)//THIS FUNCTION SETS THE WINDOW SIZE AND THE WINDOW CORRECTION
{
    header->windowSize = 3;
    header->windowCorrection = 1;
}

void initSynAck(custom_header *header)//THIS FUNCTION INITIALIZES THE SYN_ACK PACKET USING A STRUCT CALLED HEADER. IN OUR CASE THE HEADER POINTER IS A POINTER TO THE WHOLE PACKET
{
    header->flag = FLAG_SYN_ACK;
    header->seqNum = 2;
    header->unCalculatedCheckSum = 0;
    header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));
}

void sendTo(int socket_desc, char server_message[2000], struct sockaddr_in client_addr, int client_struct_length)//THIS IS THE SENDTO FUNCTION WITH OUR MODIFIED SIMULATOR THAT SIMULATES LOST PACKETS AND CORRUPT ONES.
{
    int rnd = rand() % 10 + 1;//A RANDOM FUNCTION IS USED TO RANDOMIZE ERRORS
   if(rnd >= 8)
    {
        if (sendto(socket_desc, server_message, sizeof(server_message), 0,
         (struct sockaddr*)&client_addr, client_struct_length) < 0){
        printf("Can't send, UNKNOWN ERROR...\n");
        printf("Terminating Program...\n");
        exit(EXIT_FAILURE);
        
        }
    }
    
}

void clear(char server_message[2000], char client_message[2000], custom_header *header)//THIS FUNCTION CLEARS THE HEADER POINTER, PACKET AND THE BUFFERS 
{
    memset(server_message, 0, sizeof(server_message));//CLEARS HEADER SERVER MESSAGE BUFFER
    memset(client_message, 0, sizeof(client_message)); //CLEARS HEADER CLIENT MESSAGE BUFFER
    memset(header, 0, sizeof(header)); //CLEARS HEADER POINTER
}

void copyHeadToBuff(custom_header *header, char client_message[2000])//THIS FUNCTION IS USED TO COPY THE HEADER TO THE BUFFER BEFORE SENDING OF THE PACKET
{
    memcpy(client_message, header, sizeof(client_message));
}

void copyBuffToHead(custom_header *header, char client_message[2000])//THIS FUNCTION IS USED TO COPY THE BUFFER TO THE HEADER FOR US TO BE ABLE TO READ THE HEADER AND PACKET INFORMATION
{
    memcpy(header, client_message, sizeof(client_message));
    
}


int definePacket(custom_header *header)//THIS FUNCTION DEFINES WHAT KIND OF PACKET THAT WE HAVE RECEIVED. 
{
    int cSum = 0;
    cSum = header->checkSum;
    header->checkSum = 0;
    if(cSum == calculateChecksum((char*)header, sizeof(custom_header)))//MAKES SURE THAT THE CHECKSUM IS EXPECTED ONE
    {
        
        if(header->flag == FLAG_FIN && header->seqNum == 1)//CHECKS IF THE FLAG IS A FIN PACKET AND IF THE SEQUENCE NUMBER IS 1
        {
            return GOT_FIN; //RETURNS THE ENUMERATED TYPE GOT_FIN
        }
        if(header->flag == FLAG_FIN_ACK && header->seqNum == 2) //CHECKS IF THE FLAG IS A FIN_ACK PACKET AND IF THE SEQUENCE NUMBER IS 2
        {
            return GOT_FIN_ACK; //RETURNS THE ENUMERATED TYPE GOT_FIN_ACK
        }
        if(header->flag == FLAG_SYN && header->seqNum == 1) //CHECKS IF THE FLAG IS A SYN PACKET AND IF THE SEQUENCE NUMBER IS 1
        {
            return GOT_SYN; //RETURNS THE ENUMERATED TYPE GOT_SYN
        }
        if(header->flag = FLAG_SYN_ACKACK && header->seqNum == 3) //CHECKS IF THE FLAG IS A SYN_ACKACK PACKET AND IF THE SEUENCE NUMBER IS 3
        {
            return GOT_ACK; //RETURNS THE ENUMERATED TYPE GOT_ACK
        }
        else //THE PACKET IS CORRUPT
        {
            
            return GOT_CORRUPT; //RETURNS THE ENUMERATED TYPE GOT_CORRUPT
        }
    }
    else
    {
        return GOT_CORRUPT; //RETURNS THE ENUMERATED TYPE GOT_CORRUPT
    }
}

int slidingDefine(custom_header *header, int arr[3])//THIS IS THE FUNCTION DEFINING WHAT KIND OF PACKETS WE RECEIVE IN THE SLIDING WINDOW PROTOCOL
{
    int event = 0;
    
    int cSum = 0;
    cSum = header->checkSum;
    header->checkSum = 0;
    if(cSum == calculateChecksum((char*)header, sizeof(custom_header))) //MAKES SURE THAT THE CHECKSUM IS EXPECTED ONE
    {
        
        if(header->flag == FLAG_FIN && header->seqNum == 1) //CHECKS IF THE FLAG IS A FIN PACKET AND IF THE SEQUENCE NUMBER IS 1. WE CHECK A FIN HERE AS WELL SINCE DURING PACKET LOSS THE PROGRAM MIGHT RECEIVE ACKS FROM THE FIRST "SEND-ACK" FUNCTION IN THE SLIDING WINDOW.
        {
            return GOT_FIN;
        }

        if(header->flag == FLAG_PACKET && header->seqNum == 1) //CHECKS IF IT IS THE EXPECTED PACKET
        {
            //HERE WE USE AN ARRAY TO KEEP TRACK OF THE PACKETS WE RECEIVE
            //BUY USING AN ARRAY WE ALSO CAN USE CONDITIONALS TO CHECK IF WE RECEIVE OUR PACKETS IN ORDER OR NOT. 
            if(arr[0]==1)
            {
                printf("\nFirst Packet was received again!\n");

            }
            else if(arr[0] == 0 && arr[1] == 0 && arr[2] == 0) //IF THE ARRAY IS EMPTY IT MEANS THAT ITS THE FIRST RECEIVED INSTANCE OF THE PACKET. IF NOT IT MEANS THAT THE PACKET RECEIVED JUST NEEDS AN AKC SINCE IT MIGHT HAVE BEEN LOST EARLIER.
            {
                
                arr[0] = 1; 
                printf("\nFirst Packet was received in order!\n");
                event = RECV1_SLIDE;
                return event;
                
                

            }
            else if(arr[1] == 2 && arr[2] == 3) //HERE WE CAN SLIDE THE ENTIRE WINDOW SINCE THE FIRST PACKET ARRIVED AFTER BOTH THE SECOND AND THE THIRD.
            {
                arr[0] = 1;
                
                event = RECV1_SLIDE;
                return event;
               
            }
            else if(arr[1] == 2)
            {
                arr[0] = 1;
                event = RECV1_SLIDE;
                return event;
                
            }
            
        }
        if(header->flag == FLAG_PACKET && header->seqNum == 2)//CHECKS IF ITS THE SECOND PACKET EXPECTED
        {
            //SAME CONDITIONALS ARE USED HERE. 
            //IF THE FIRST PACKET IS NOT RECEIVED WE SAVE THE SECOND PACKET IN THE ARRAY AND WAIT FOR THE FIRST PACKET TO ARRIVE BEFORE SLIDING. 
            if(arr[1] == 2)//THIS MEANS THAT AN ACK OF THIS ONE NEEDS TO BE RESENT SINCE IT WAS LOST EARLIER
            {
                printf("Second packet was received again!\n");
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
                printf("\nSecond Packet was received in order!\n"); //SECOND PACKET WAS RECEIVED IN ORDER
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
        if(header->flag == FLAG_PACKET && header->seqNum == 3) //CHECKS IF ITS THE THIRD PACKET EXPECTED
        {
            //SAME CONDITIONALS ARE USED HERE.
            if(arr[2] == 3)
            {
                //resending ack of packet 3.
            }
            if(arr[0] == 1 && arr[1] == 2 )
            {
                printf("\nThird Packet was received in order!\n");
                arr[2] = 3;
                event = RECV3_SLIDE;
                return event;
                //slide 3
            }
            else if(arr[1] == 2)//Betyder att p1 ej finns
            {
                arr[2] = 3; //väntar på p1
                //No slide
                event = RECV3_SLIDE;
                return event;
            }
            else if (arr[0] == 1) //Betyder att p2 ej finns
            {
                arr[2] = 3; //väntar på p2
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

    //HERE A CORRUPT PACKET WAS RECEIVED -> FULL RETRANSMISSION OF PACKETS
    else
    {
        return GOT_CORRUPT; //THIS MEANS THAT THE PACKET DIDNT HAVE THE SAME CHECKSUM WHICH MEANS THAT IT IS CORRUPT. 
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

int timer(int socket_desc, char client_message[2000], struct sockaddr_in client_addr, int client_struct_length, custom_header *header) //THIS FUNCTION IS THE TIMER FUNCTION THAT USES THE READABLE_TIMEO FUNCTION TO WAIT FOR A SPECIFIED AMOUNT OF TIME BEFORE RETURNING A VALUE.
{
    int event = 0;
    int count = readable_timeo(socket_desc, 3);
    if(count > 0)
    {
        if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
        (struct sockaddr*)&client_addr, &client_struct_length) < 0){
        printf("Couldn't receive\n");
        return -1;
        }
        //IF THESE THREE LINES ARE REACHED IT MEANS THAT THE CLIENT MESSAGE WAS RECEIVED AND THE HEADER IS COPIED TO THE HEADER STRUCTURE.
        memcpy(header, client_message, sizeof(client_message));//copies the client message to the header;
        event = definePacket(header); //CALLING THE "DEFINE-PACKET" FUNCTION TO SEE WHAT PACKET WE RECEIVED.
        return event;
    }
    else if(count == 0)
    {
        //Resendning...
        return event; // THIS MEANS THAT THE TIMER RAN OUT AND THE PACKET WAS NOT RECEIVED. HERE WE RETURN A 0 VALUE WHICH MEANS THAT THE PACKET WAS NOT RECEIVED.
    }
    

}

int slidingTimer(int socket_desc, char client_message[2000], struct sockaddr_in client_addr, int client_struct_length, custom_header *header, int arr[3])//THIS IS ANOTHER TIMER FUNCTION USED BY THE SLIDING WINDOW PROTOCOL AND IS ACTUALLY THE SAME AS THE OTHER TIMER FUNCTION.
{ //THIS ONE ALSO USES THE READABLE_TIMEO FUNCTION TO WAIT FOR A SPECIFIED AMOUNT OF TIME BEFORE RETURNING A VALUE.
    int event = 0;
    int count = readable_timeo(socket_desc, 4);
    
    if(count > 0)
    {
        if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
        (struct sockaddr*)&client_addr, &client_struct_length) < 0){
        printf("Couldn't receive\n");
        return -1;
        }
    
        //THESE THREE ROWS ARE EXECUTED IF WE HAVE RECEIVED SOMETHING
        memcpy(header, client_message, sizeof(client_message));
        event = slidingDefine(header, arr); //CALLING THE "SLIDING-DEFINE" FUNCTION TO SEE WHAT PACKET WE RECEIVED.
        return event;
    }
    if(count == 0)
    {
        //Here a packet that is expected never arrived, this will lead to a full retransmission of the packets from the client side.
        event = 0;
        return event;
    }
}




int main(void){
    int socket_desc; //SOCKET
    int event = 0;
    int state = WAIT_SYN; //STARTING STATE OF THE PROGRM == WAITING FOR A CONNECTION REQUEST
    bool setup = false; //BOOLEAN VARIABLE USED TO CHECK IF THE CONNECTION HAS BEEN ESTABLISHED
    bool procedure = false; //BOOLEAN VARIABLE USED TO CHECK IF THE PROCEDURE HAS BEEN COMPLETED
    bool termination = false; //BOOLEAN VARIABLE USED TO CHECK IF THE TERMINATION HAS BEEN COMPLETED
    struct sockaddr_in server_addr, client_addr; //SOCKADDR_IN STRUCTURES
    char server_message[2000], client_message[2000]; //STRINGS TO HOLD MESSAGES FROM CLIENT AND SERVER. THESE ARE ALSO REFERRED TO AS OUR BUFFERS IN THE REPORT.
    int client_struct_length = sizeof(client_addr); //LENGTH OF THE CLIENT STRUCTURE
    custom_header *header = (custom_header*)malloc(sizeof(custom_header)); //HEADER POINTER POINTING TO OUR PACKET INFORMATION WITH ALLOCATED MEMORY USING MALLOC FOR THE POINTER
    
    clear(server_message, client_message, header); //CLEARING HEADER, PACKET AND BUFFERS BEFORE PROGRAM START
    // Create UDP socket:
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //CREATING UDP SOCKET USING THE SOCKET FUNCTION AND DGRAM AS THE SOCKET TYPE
    
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");
    
    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Bind to the set port and IP:
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");
    
    printf("Listening for incoming messages...\n\n");


//RECEIVING THE SYN:
//THE FIRST RECEIVE FUNCTION DOES NOT NEED A TIMER SINCE THE SERVER ALWAYS IS WAITING FOR A MESSAGE AS SOON AS THE SERVER PROGRAM IS LAUCHED. 
    while(1)
    {
        if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
            (struct sockaddr*)&client_addr, &client_struct_length) < 0){
            printf("Couldn't receive\n");
            return -1;
        }
        memcpy(header, client_message, sizeof(client_message));//copies the client message to the header;
        event = definePacket(header);
        break;
    }
        

    
    //CONNECTION SETUP SERVER PROCEDURE:
    while(setup == false) //WHILE LOOP WITH BOOLEAN VARIABLE SETUP AS CONDITION
    {
        switch (state) //SWITCH CASE STATEMENT WITH STATE AS THE VARIABLE
        {
            case WAIT_SYN : //WAITING FOR A SYN PACKET
                if(event == GOT_SYN) 
                {
                    //HERE WE HAVE RECEIVED A SYN PACKET AND WE ARE NOW SENDING A SYN ACKNOWLEDGEMENT
                    printf("A connection request was received\n");
                    printf("Sending Syn Acknowledgement\n");
                    clear(server_message, client_message, header); //CLEARING HEADER, PACKET AND BUFFERS
                    initSynAck(header); //INITIALIZING THE SYN ACKNOWLEDGEMENT PACKET
                    copyHeadToBuff(header, server_message); //COPYING THE HEADER TO THE SERVER MESSAGE BUFFER
                    //sleep(1);
                    sendTo(socket_desc, server_message, client_addr, client_struct_length); //SENDING THE SERVER MESSAGE BUFFER TO THE CLIENT
                    
                    event = timer(socket_desc, client_message, client_addr, client_struct_length, header); //CALLING THE TIMER FUNCTION TO WAIT FOR A RESPONSE FROM THE CLIENT
                    if(event == 0) //ASSUMING LOST PACKET OF THE SYN ACKNOWLEDGEMENT ACK IF THE TIMER RETURNS 0 
                    {
                        
                        printf("Syn acknowledgement was either lost or received assuming connection!\n");
                        sleep(2);
                        state = WAIT_ACK_SYN_ACK; //GOING TO THE NEXT STATE
                        event = GOT_ACK; //ASSUMING THAT THE CLIENT RECEIVED THE SYN ACKNOWLEDGEMENT AND IS SENDING AN ACKNOWLEDGEMENT ACKNOWLEDGEMENT
                    }
                    else
                    {
                        state = WAIT_ACK_SYN_ACK;//GOING TO THE WAIT FOR ACK OF THE ACK STATE.
                    }
                    
                }
                else if(event == GOT_CORRUPT)//HERE A CORRUPT PACKET WAS RECEIVED 
                {
                    clear(server_message, client_message, header);
                    printf("A corrupt packet has been received, probably a corrupt ACK-ACK!\n");//ENTERING THE TIMEOUT STAGE BEOFRE CONNECTING.
                    printf("Entering timeout...\n");
                    sleep(3);//Timeout 
                    event = GOT_SYN;
                    state = WAIT_SYN;
                    
                }
                break;

            case WAIT_ACK_SYN_ACK : //THIS IS THE STATE WHERE WE ARE WAITING FOR THE ACKNOWLEDGEMENT ACKNOWLEDGEMENT
                if(event == GOT_ACK)
                {
                    printf("Final Acknowledgement received\n");//IF THE EVENT IS == TO GOT_ACK THEN WE HAVE RECEIVED THE ACKNOWLEDGEMENT ACKNOWLEDGEMENT
                    printf("Establishing connection...\n");
                    //sleep(2);
                    printf("\n\nConnection established!\n\n");
                    setup = true;//SETTING THE BOOLEAN VARIABLE SETUP TO TRUE SINCE THE CONNECTION HAS BEEN ESTABLISHED
                    //exit(EXIT_SUCCESS);
                }
                if(event == GOT_CORRUPT)
                {
                    printf("A corrupt packet has been received\n");//HERE WE GOT A CORRUPT ACK ACKNOWLEDGEMENT, BUT AS ITS THE LAST PACKET IN THE THREEWAY-HANDSHAKE WE ASSUME CONNECTION
                    printf("Assuming an established connection...\n");
                    sleep(4);//Timeout
                    printf("Connection established!");
                    setup = true;
                }
        }
    }

    
    procedure = false;
    event = 0;
    state = SET_WINDOW_SIZE;
    int arr[3] = {0};//FOR THE SLIDING WINDOW PROTOCOL, WE ARE USING AN ARRAY TO KEEP TRACK OF THE PACKETS RECEIVED.
    int arr2[3] = {0};//WE HAVE TO ARRAYS, ONE FOR FILLING THE PACKETS RECEIVED AND ANOTHER ONE WITH THE SAME DATA THAT WE CHECK EXPECTED PACKETS ETC. 
    int ackNum = 1;
    //SERVER SLIDING WINDOW PROTOCOL:
    while(procedure == false)
    {
        switch (state)
        {
            case SET_WINDOW_SIZE : //HERE WE SET THE WINDOW SIZE TO 3
                printf("\nSetting window size...\n");
                sleep(1);
                clear(server_message, client_message, header);
                setWindowSize(header);//SETTING THE WINDOW SIZE WITH THE HELP FROM THE HEADER POINTER TO THE PACKET
                state = RECV1;//ENTERING RECEIVING STATE
            break;

            case RECV1 :
                printf("\nReceiving...\n\n");
                event = slidingTimer(socket_desc, client_message, client_addr, client_struct_length, header, arr2);
                if(event == RECV1_SLIDE)//HERE A FIXED VARIABLE IS RETURNED FROM THE "SLIDING-DEFINE" FUNCTION WHICH IS USED TO CHECK WHAT KIND OF PACKET THAT WAS RECEIVED. 
                {
                    arr[0] = 1;
                    printf("First packet received!\n\n");
                    if(arr[0] == 1 && arr[1] == 2 && arr[2] == 3)//HERE WE CHECK IF ALL PACKETS HAVE BEEN RECEIVED, SINCE PACKETS CAN BE RECEIVED IN DIFF ORDER.
                    {//IF SO WE SLIDE THE WINDOW AND SEND ALL ACKS
                        printf("All packets received!\n");
                        printf("Sliding...\n");
                        state = SEND_ALL_ACKS;
                        sleep(2);
                        //Change to state send all acks
                    }
                    

                    
                    
                }
                if(event == RECV2_SLIDE)//SAME HERE AS BEFORE BUT FOR THE SECOND PACKET
                {
                    arr[1] = 2;
                    printf("Second packet received!\n\n");
                    if(arr[0] == 1 && arr[1] == 2 && arr[2] == 3)
                    {
                        printf("All packets received!\n");
                        printf("Sliding...\n");
                        state = SEND_ALL_ACKS;
                        sleep(2);
                        //Change to state send all acks
                    }
                    
                    
                    
                }
                if(event == RECV3_SLIDE)//SAME HERE AS BEFORE BUT FOR THE THIRD PACKET
                {
                    arr[2] = 3;
                    printf("Third packet received!\n\n");
                    if(arr[0] == 1 && arr[1] == 2 && arr[2] == 3)
                    {
                        printf("All packets received!\n");
                        printf("Sliding...\n");
                        state = SEND_ALL_ACKS;
                        sleep(2);
                        //Change to state send all acks
                    }
                    
                    
                }

                if(event == GOT_CORRUPT)//CONDITION TO CHECK IF A CORRUPT PACKET WAS RECEIVED
                {
                
                    printf("A corrupt packet has been received in the sliding window!\n");
                    printf("Waiting for a full retransmission...\n");//WHAT WE DO HERE IS TO WAIT FOR A FULL RETRANSMISSION OF THE PACKETS FROM THE CLIENT SIDE
                    sleep(4);//THIS TIMEOUT WILL INDICATE THE CLIENT THAT IT SHOULD RESEND THE PACKETS
                }

                if(event == 0)//SAME WHEN A PACKET IS LOST. 
                {
                    printf("A packet was lost, waiting for a full retransmission...\n");
                    sleep(2);
                    clear(server_message, client_message, header);
                }

                

                
                
            break;

            

            case SEND_ALL_ACKS ://THIS IS THE SEND ALL ACKS STATE, HERE WE SEND ALL ACKNOWLEDGEMENTS TO THE CLIENT SIDE
                printf("\nSending all acknowledgements...\n");
                sleep(1);
                clear(server_message, client_message, header); 
                initAck(header, ackNum, socket_desc, server_message, sizeof(server_message), client_addr, client_struct_length); //INITIALIZING THE ACKNOWLEDGEMENT PACKET WITH THE HELP OF THE HEADER POINTER
                clear(server_message, client_message, header);
                sleep(1);
                memset(arr, 0, sizeof(arr)); //CLEARING THE ARRAYS TO BE ABLE TO RECEIVE NEW PACKETS
                memset(arr2, 0, sizeof(arr2)); //CLEARING THE ARRAYS TO BE ABLE TO RECEIVE NEW PACKETS
                event = slidingTimer(socket_desc, client_message, client_addr, client_struct_length, header, arr2); 
                //printf("%d\n", event);
                if(event == RECV1_SLIDE || event == RECV2_SLIDE || event == RECV3_SLIDE)
                {
                    if(event == RECV1_SLIDE){arr[0] = 1;}//FILLING ARRAYS WITH RECEIVED ACKS
                    if (event == RECV2_SLIDE){arr[1] = 2;}//FILLING ARRAYS WITH RECEIVED ACKS
                    if(event == RECV3_SLIDE){arr[2] = 3;}//FILLING ARRAYS WITH RECEIVED ACKS
                    state = SLIDE;
                }

                if(event == GOT_FIN)//HERE WE CHECK IF A FIN PACKET WAS RECEIVED. THIS IS A POSSIBLE OUTCOME SINCE THE CLIENT MIGHT WANT TO END CONNECTION WHENEVER.
                {
                    state = SEND_FIN_ACK;
                    procedure = true;//HERE WE SET THE PROCEDURE VARIABLE TO TRUE TO BE ABLE TO EXIT THE WHILE LOOP
                }

                if(event == 0)//IF NOTHING RECEIVED THE SERVER RESENDS ALL ACKS UNTIL IT GETS A NOTIFICATION FROM THE CLIENT OF SOMETHING ELSE. 
                {
                    //SLIDE
                    //state = SLIDE;
                        //initAck(header, ackNum, socket_desc, server_message, sizeof(server_message), client_addr, client_struct_length);
                        //printf("Sliding...\n");
                        state = SEND_ALL_ACKS;
                    
                    
                }
                if(event == NAK)//CHECKING THE POSSIBILITY OF A RECEIVED NAK FROM THE CLIENT MEANING THAT SOMEHTING WENT WRONG DURING TRANSMISSION. 
                {
                    printf("Acks sent were either lost or corrupted during transmission!\n");
                    printf("Preparing a full retransmission...\n");
                    
                    int count = 1;
                    int acknumber = 1;
                    for(int i = 0; i < 3; i++)//THIS FOR LOOP RESENDS THE ACKS AGAIN
                    {
                        
                        header->ackNum = acknumber;
                        header->flag = FLAG_ACK;
                        header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));
                        memcpy(server_message, header, sizeof(custom_header));
                        printf("%d\n", header->checkSum);
                        sendTo(socket_desc, server_message, client_addr, client_struct_length);
                        printf("Ack %d sent!\n", count);
                        clear(server_message, server_message, header);
                        sleep(2);
                        acknumber++;
                        count++;

                    }

                    event = timer(socket_desc, client_message, client_addr, client_struct_length, header);
                    state = CHECK;
                    
                    
                }
            break; 

            case SLIDE : //THIS IS THE SLIDE STATE, HERE WE SLIDE THE WINDOW AND WAIT FOR THE NEXT SET OF PACKETS TO BE RECEIVED
            //THIS STATE IS A DUPLICATE OF THE EARLIER WINDOW STATE, WE JUST REPEAT THE PROCESS ONCE MORE TO SHOW THAT THE PROGRAM CAN HANDLE A SLIDING EVENT.
            sleep(1);
            clear(server_message, client_message, header);
            

                printf("\nReceiving...\n\n");
                event = slidingTimer(socket_desc, client_message, client_addr, client_struct_length, header, arr2);
                if(event == RECV1_SLIDE)
                {
                    arr[0] = 1;
                    printf("First packet received!\n\n");
                    if(arr[0] == 1 && arr[1] == 2 && arr[2] == 3)
                    {
                        printf("All packets received!\n");
                        printf("Sliding...\n");
                        state = SLIDE_SEND_ACKS;
                        sleep(2);
                        
                    }
                    
                    

                    
                    
                }
                if(event == RECV2_SLIDE)
                {
                    arr[1] = 2;
                    printf("Second packet received!\n\n");
                    if(arr[0] == 1 && arr[1] == 2 && arr[2] == 3)
                    {
                        printf("All packets received!\n");
                        printf("Sliding...\n");
                        state = SLIDE_SEND_ACKS;
                        sleep(2);
                        
                    }
                    
                    
                    
                    
                }
                if(event == RECV3_SLIDE)
                {
                    arr[2] = 3;
                    printf("Third packet received!\n\n");
                    if(arr[0] == 1 && arr[1] == 2 && arr[2] == 3)
                    {
                        printf("All packets received!\n");
                        printf("Sliding...\n");
                        state = SLIDE_SEND_ACKS;
                        sleep(2);
                        //Change to state send all acks
                    }
                    
                    
                    
                }

                if(event == GOT_CORRUPT)
                {
                
                    printf("A corrupt packet has been received in the sliding window!\n");
                    printf("Waiting for a full retransmission...\n");
                    sleep(2);
                }

                if(event == 0)
                {
                    printf("A packet was lost, waiting for a full retransmission...\n");
                    sleep(2);
                    clear(server_message, client_message, header);
                }

                

                
                
            break;
            

            case SLIDE_SEND_ACKS : //SIMILAR TO THE "SEND-ALL-ACK" STATE THAT WE USED EARLIER THIS ONE SENDS THE ACKS OF THE RECEIVED PACKETS BACK TO THE CLIENT AND WORKS EXACLTY IN THE SAME WAY.

                printf("\nSending all acknowledgements...\n");
                sleep(1);
                clear(server_message, client_message, header);
                initAck(header, ackNum, socket_desc, server_message, sizeof(server_message), client_addr, client_struct_length);
                clear(server_message, client_message, header);
                //sleep(1);
                event = timer(socket_desc, client_message, client_addr, client_struct_length, header); 
                printf("%d\n", event);
                if(event == GOT_FIN)
                {
                    state = SEND_FIN_ACK;
                    procedure = true;
                }
                if(event == 0)
                {
                        
                        state = SLIDE_SEND_ACKS;
                    
                }
                if(event == NAK)
                {
                    printf("Acks sent were either lost or corrupted during transmission!\n");
                    printf("Preparing a full retransmission...\n");
                    
                    int count = 1;
                    int acknumber = 1;
                    for(int i = 0; i < 3; i++)
                    {
                        
                        header->ackNum = acknumber;
                        header->flag = FLAG_ACK;
                        header->checkSum = calculateChecksum((char*)header, sizeof(custom_header));
                        memcpy(server_message, header, sizeof(custom_header));
                        printf("%d\n", header->checkSum);
                        sendTo(socket_desc, server_message, client_addr, client_struct_length);
                        printf("Ack %d sent!\n", count);
                        clear(server_message, server_message, header);
                        sleep(2);
                        acknumber++;
                        count++;

                    }

                    event = timer(socket_desc, client_message, client_addr, client_struct_length, header);
                    state = CHECK;
                    
                    
                }
            break; 




            case CHECK : //AN EXTRA CHECK STATE TO CHECK INFO ABOUT THE RECEIVED PACKETS AND IF THERE IS MORE TO BE RECEIVED.
                if(event == 0)
                {
                    procedure = true;
                    state = WAIT_FIN;
                }
                else
                {
                    state = SEND_ALL_ACKS;
                }
            break;
        }
    }
    




    //SERVER CONNETION TEARDOWN STATE MACHINE
    bool finish = false;
    int finCounter = 0;
    while(finish == false)
    {
        
        
        switch(state)
        {
            //HERE THE SERVER WAITS FOR A FIN PACKET FROM THE CLIENT
            case WAIT_FIN :
                clear(server_message, client_message, header);
                event = timer(socket_desc, client_message, client_addr, client_struct_length, header);
                if(event == GOT_FIN)//RECEIVED FIN REQUEST
                {
                    printf("A teardown request has been received!\n");
                    state = SEND_FIN_ACK;
                }
                if(event == GOT_CORRUPT)//CORRUPT PACKET RECEIVED
                {
                    printf("A corrupt packet has been received!\n");
                    printf("Waiting on a retransmission...\n");
                    sleep(2);
 
                }
                if(event == 0)//NOTHING RECEIVED == TERMINATION OF PROGRAM SINCE THE CONNECTION CANNOT BE INFINITELY KEPT ALIVE.
                {
                    //HERE WE USE A COUNTER TO SEE HOW MANY TIMES THE SERVER HASNT RECEIVED ANYTHING FROM THE CLIENT. IF THE COUNTER REACHES 3 THEN THE SERVER TERMINATES THE PROGRAM.
                    finCounter++;
                    if(finCounter == 3)
                    {
                        printf("No teardown request received, terminating program...\n");
                        exit(EXIT_SUCCESS);
                    }
                    printf("Expected packet was lost!\n");
                    printf("Waiting on a retransmisison...\n");
                    sleep(2); 
                }
            break;

            case SEND_FIN_ACK : //HERE WE HAVE RECEIVED A FIN AND HAVE MOVED TO THE SENDING OF THE FIN ACK. 
                printf("Sending FIN ACK!\n");
                clear(server_message, client_message, header);
                initFinAck(header);
                memcpy(server_message, header, sizeof(custom_header));
                sendTo(socket_desc, server_message, client_addr, client_struct_length);
                clear(server_message, client_message, header);
                event = timer(socket_desc, client_message, client_addr, client_struct_length, header);

                if(event == GOT_FIN_ACKACK || event == 0)//IF THE FIN ACK ACK IS RECEIVED OR NOTHING IS RECEIVED AFTER THE FIN ACK HAS BEEN SENT THE SERVER ASSUMES A TEARDOWN EVENT AND TERMINATES THE PROCESS.
                {
                    printf("\nTerminating program... Goodbye!\n");
                    finish = true;
                    exit(EXIT_SUCCESS);
                }
                else if(event == 0)
                {
                    sleep(2);
                    printf("\n Assuming teardown complete, goodbye!\n");
                    finish = true;
                    exit(EXIT_SUCCESS);
                }
            break;
                
                
        }
    }


       

    return 0;
}
