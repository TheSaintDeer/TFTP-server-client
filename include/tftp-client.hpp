#ifndef CLIENT
#define CLIENT

#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "packet.hpp"

using namespace std;

/**
 * Input parametrs
*/
struct parametrs {
    char* hostname;
    int port = STANDART_PORT;
    char* filepath;
    char* dest_filepath;
};

/**
 * Accept and process input parameters to start the client
*/
void get_parametrs(struct parametrs* p, int* oper, int argc, char **argv);

void get_respones_RRQ(int sock, sockaddr_in server, struct parametrs p);

void get_respones_RRQ(int sock, sockaddr_in server, struct parametrs p);

#endif