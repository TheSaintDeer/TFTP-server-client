#ifndef PACKET
#define PACKET


#include <netdb.h>
#include <stdio.h>
#include <unistd.h>





#include <string.h>
#include <stdlib.h>

#include <netinet/in.h>

#define PACKET_SIZE    516
#define DATA_SIZE      512
#define MODE            "octet"
#define READ            1
#define WRITE           2
#define DATA            3
#define ACK             4
#define ERROR           5
#define STANDART_PORT   69

void send_RRQ(int sock, char* filepath, struct sockaddr_in server);

void send_WRQ(int sock, char* filepath, struct sockaddr_in server);

void send_DATA(int sock, uint16_t packet_number, char* data, struct sockaddr_in server);

void send_ACK(int sock, uint16_t packet_number, struct sockaddr_in server);

void send_ERR(int sock, uint16_t error_code, char* error_msg, struct sockaddr_in server);

#endif