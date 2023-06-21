//AUHTOR: LIAM BURBERRY GAHM, MAX STRANG
//Server.H file.


#ifndef SERVER_H
#define SERVER_H

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

typedef struct{
    uint16_t checkSum;
    uint8_t flag;
    uint16_t seqNum;
    uint16_t ackNum;
    uint16_t data;
    uint16_t unCalculatedCheckSum; 
    int16_t windowSize;
    uint16_t windowCorrection;
    //uint16_t NAK2;
}custom_header;

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
void initFinAck(custom_header *header);
void initFin(custom_header *header);
void initFinAckAck();
void initAck(custom_header *header, int ackNumber, int socket_desc, char server_message[2000], socklen_t server_len, struct sockaddr_in client_addr, int client_struct_length);
void setWindowSize(custom_header *header);
void initSynAck(custom_header *header);
void sendTo(int socket_desc, char server_message[2000], struct sockaddr_in client_addr, int client_struct_length);
void clear(char server_message[2000], char client_message[2000], custom_header *header);
void copyHeadToBuff(custom_header *header, char client_message[2000]);
void copyBuffToHead(custom_header *header, char client_message[2000]);
int definePacket(custom_header *header);
int slidingDefine(custom_header *header, int arr[3]);
int readable_timeo(int fd, int sec);
int timer(int socket_desc, char client_message[2000], struct sockaddr_in client_addr, int client_struct_length, custom_header *header);
int slidingTimer(int socket_desc, char client_message[2000], struct sockaddr_in client_addr, int client_struct_length, custom_header *header, int arr[3]);


#endif
