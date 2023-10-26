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
#define RRQ            1
#define WRQ           2
#define DATA            3
#define ACK             4
#define ERROR           5
#define STANDART_PORT   69

#define NOT_DEFINED             0
#define FILE_NOT_FOUND          1
#define ACCESS_VIOLATION        2
#define DISK_FULL               3
#define ILLEGAL_TFTP_OPERATION  4
#define UNKNOWN_TRANSFER_ID     5
#define FILE_ALREADY_EXIST      6
#define NO_SUCH_USER            7

void send_RRQ(int sock, char* filepath, struct sockaddr_in sockad);

void send_WRQ(int sock, char* filepath, struct sockaddr_in sockad);

void send_DATA(int sock, uint16_t packet_number, char* data, struct sockaddr_in sockad);

void send_ACK(int sock, uint16_t packet_number, struct sockaddr_in sockad);

void send_ERR(int sock, uint16_t error_code, struct sockaddr_in sockad);

#endif