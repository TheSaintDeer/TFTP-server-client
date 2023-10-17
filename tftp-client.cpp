#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>

#include "packet.hpp"

using namespace std;

int main(int argc, char **argv) {

    char* hostname = "";
    char* port = "";
    char* filepath = "";
    char* dest_filepath = "";
    int c;

    while ((c = getopt (argc, argv, ":h:p:f:t:")) != -1) {
        switch (c) {
            case 'h':
                hostname = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'f':
                filepath = optarg;
                break;
            case 't':
                dest_filepath = optarg;
                break;
            case '?':
                if (optopt == 'h' || optopt == 't') 
                    fprintf(stderr, "Option -%c requires an argument\n", optopt);
                else
                    fprintf(stderr, "Unknown option\n");
	  	        
	            exit(1);
        }
    }

    // cout << hostname << port << filepath << dest_filepath << endl;


    int sock;
    struct sockaddr_in server;
    struct hostent *host;
    char buffer[PAYLOAD_SIZE], *p;
    int count;
    socklen_t server_len;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    host = gethostbyname(hostname);

    if (host == NULL) {
        fprintf(stderr, "unknown host: %s\n", argv[1]);
        exit(2);
    }

    server.sin_family = AF_INET;
    memcpy(&server.sin_addr.s_addr, host->h_addr, host->h_length);
    server.sin_port = htons(atoi(port));

    *(short *)buffer = htons(WRITE);	/* The op-code   */
    p = buffer + 2;
    strcpy(p, filepath);			/* The file name */
    p += strlen(filepath) + 1;	/* Keep the nul  */
    strcpy(p, MODE);			/* The Mode      */
    p += strlen(MODE) + 1;

    count = sendto(sock, buffer, p-buffer, 0, (struct sockaddr *)&server, sizeof(server));

    do {
        server_len = sizeof(server);
        count = recvfrom(sock, buffer, PAYLOAD_SIZE, 0, (struct sockaddr *)&server, &server_len);
        if (ntohs(*(short *)buffer) == ERROR) {
            fprintf(stderr, "rcat: %s\n", buffer+4);
        } else {
            write(1, buffer+4, count-4);
            *(short *)buffer = htons(ACK);
            sendto(sock, buffer, 4, 0, (struct sockaddr *)&server, sizeof server);
        }
    } while (count == 512);

    return 0; 
}