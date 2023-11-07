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

using namespace std;

struct parametrs {
    int port = STANDART_PORT;
    char* root_dirpath;
};

/**
 * Accept and process input parameters to start the client
*/
void get_parametrs(struct parametrs* p, int argc, char **argv);

void main_loop(struct parametrs p);

void proccessing_RRQ(int sock, sockaddr_in client, size_t addr_len, char* buffer);

void proccessing_WRQ(int sock, sockaddr_in client, size_t addr_len, char* buffer);

void RRQ_octet(int sock, sockaddr_in client, size_t addr_len, struct opts o, FILE* src_file, bool with_opts);

void RRQ_netascii(int sock, sockaddr_in client, size_t addr_len, struct opts o, FILE* src_file, bool with_opts);

void WRQ_communication(int sock, sockaddr_in client, size_t addr_len, struct opts o, FILE* src_file, bool with_opts);

#endif