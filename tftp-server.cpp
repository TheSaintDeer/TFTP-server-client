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

int main(int argc, char **argv) {

    int port = 69;
    string root_dirpath = argv[argc-1];

    int c;

    while ((c = getopt (argc, argv, "p:")) != -1) {
        switch (c) {
            case 'p':
                port = atoi(optarg);
                break;
        }
    }

    int sock;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr; 

    if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    }

    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if ( bind(sock, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {

        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    socklen_t len;
    int n;

    len = sizeof(cliaddr);

    n = recvfrom(sock, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
    buffer[n] = '\0'; 

    sendto(sock, (const char *)buffer, strlen(buffer), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len); 

    return 0;
}