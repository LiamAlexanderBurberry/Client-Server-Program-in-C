//AUHTOR: LIAM BURBERRY GAHM, MAX STRANG
//Client.h file.


#ifndef CLIENT_H
#define CLIENT_H

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


typedef struct {
    uint16_t checkSum;
    uint8_t flag;
    uint16_t seqNum;
    uint16_t ackNum;
    uint16_t data;
    uint16_t unCalculatedCheckSum; 
    uint16_t windowSize;
    uint16_t windowCorrection;
    //uint16_t NAK2;
} custom_header;

typedef enum{
    FLAG_NONE = 0,
    FLAG_SYN = 1,
    FLAG_SYN_ACKACK = 2,
    FLAG_SYN_ACK = 3,
    FLAG_FIN = 4,
    FLAG_FIN_ACK = 5,
    FLAG_ACK = 6,
    FLAG_PACKET = 7,
    NAK_RESEND_SYN = 8,
    NAK_RESEND_SYN_ACK = 9,
    NAK_RESEND_SYN_ACKACK = 10,
    NAK = 11,
    FLAG_FIN_ACKACK = 12
} serverFlag;
uint16_t calculateChecksum(const char *buff, size_t length);
void initFinAckAck(custom_header *header);
void initFinAck(custom_header *header);
void initFin(custom_header *header);
void initAck(custom_header *header, int ackNumber, int socket_desc, char server_message[2000], socklen_t length, struct sockaddr_in server_addr, int server_struct_length);
void initSlidingPackets(custom_header *header, int size, int socket_desc, char client_message[2000], char server_message[2000], socklen_t length, struct sockaddr_in server_addr, int server_struct_length);
void setWindowSize(custom_header *header);
void initSynAckAck(custom_header *header);
void initSyn(custom_header *header);
void clear(char server_message[2000], char client_message[2000], custom_header *header);
void copyHeadToBuff(custom_header *header, char client_message[2000]);
int definePacket(custom_header *header, char client_message[2000]);
int slidingDefine(custom_header *header, int arr[3]);
int readable_timeo(int fd, int sec);
int timer(custom_header *header, int socket_desc, char server_message[2000], socklen_t length, struct sockaddr_in server_addr, int server_struct_length);
int slidingTimer(custom_header *header, int socket_desc, char server_message[2000], socklen_t length, struct sockaddr_in server_addr, int server_struct_length, int arr[3], int recvCount);
void sendTo(int socket_desc, char client_message[2000], socklen_t length, struct sockaddr_in server_addr, int server_struct_length);


#endif
