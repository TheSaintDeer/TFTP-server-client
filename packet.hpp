#ifndef PACKET
#define PACKET
#include <iostream>

#define HEADER_SIZE     2
#define PAYLOAD_SIZE    512
#define MODE            "octet"
#define READ            1
#define WRITE           2
#define DATA            3
#define ACK             4
#define ERROR           5

typedef struct _header_ {
        u_short seq;
        u_short ack;
        u_short checksum;
        u_short offset:10;
        u_short flag:6;
} header_t;

typedef struct _packet_ {
        header_t header;
        u_char payload[PAYLOAD_SIZE];
} packet_t;

#endif