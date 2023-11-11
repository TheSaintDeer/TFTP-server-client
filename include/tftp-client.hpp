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
#include "common.hpp"

using namespace std;

int sock;

sockaddr_in server;

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

#endif