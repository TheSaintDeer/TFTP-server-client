#ifndef PACKET
#define PACKET

#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>

#define PACKET_SIZE     516
#define MODE            "octet"
#define RRQ             1
#define WRQ             2
#define DATA            3
#define ACK             4
#define ERROR           5
#define OACK            6
#define STANDART_PORT   69

struct opts {
    char* blksize;
    char* timeout;
    char* tsize;
};

char* convert_to_ASCII(char c);

char convert_from_ASCII(char* c);

void send_first_request(int sock, char* filepath, struct sockaddr_in sockad, struct opts o, int operation);

void send_DATA(int sock, uint16_t packet_number, char* data, struct sockaddr_in sockad);

void send_ACK(int sock, uint16_t packet_number, struct sockaddr_in sockad);

void send_ERR(int sock, uint16_t error_code, struct sockaddr_in sockad);

void send_OACK();

#endif