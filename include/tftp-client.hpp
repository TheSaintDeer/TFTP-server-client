#ifndef CLIENT
#define CLIENT

#include <iostream>
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
 * Structure for input parametrs
*/
struct parametrs {
    char* hostname;
    int port = STANDART_PORT;
    char* filepath;
    char* dest_filepath;
};

/**
 * Accept and process input parameters to start the client
 * @param p input parametrs
 * @param oper operation RRW or WRQ
*/
void get_parametrs(struct parametrs* p, int* oper, int argc, char **argv);

/**
 * Continuing with the RRQ operation
 * @param sock client socket file descriptor
 * @param server the socket to be used to send the packet
 * @param p input parametrs
*/
void RRQ_request(int sock, sockaddr_in server, struct parametrs p);

/**
 * Continuing with the WRQ operation
 * @param sock client socket file descriptor
 * @param server the socket to be used to send the packet
 * @param p input parametrs
*/
void get_respones_RRQ(int sock, sockaddr_in server, struct parametrs p);

#endif