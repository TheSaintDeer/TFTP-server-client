#ifndef PACKET
#define PACKET

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

void send_RRQ(int, char*, struct sockaddr_in);

void send_WRQ(int, char*, struct sockaddr_in);

void send_DATA(int, uint16_t, char*, struct sockaddr_in);

void send_ACK(int, uint16_t, struct sockaddr_in);

void send_ERR(int, uint16_t, char*, struct sockaddr_in);

#endif