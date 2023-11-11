#ifndef SERVER
#define SERVER

#include <iostream>
#include <getopt.h>
#include <string>
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include "packet.hpp"
#include "common.hpp"

using namespace std;

int sock;

struct sockaddr_in client;

struct parametrs {
    int port = STANDART_PORT;
    char* root_dirpath;
};

/**
 * Accept and process input parameters to start the client
*/
void get_parametrs(struct parametrs* p, int argc, char **argv);

void main_loop(struct parametrs p);

void processing_request(int op, size_t addr_len, char* buffer, int recv_len, struct parametrs p);

#endif