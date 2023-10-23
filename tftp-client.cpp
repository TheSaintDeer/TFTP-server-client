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
struct sockaddr_in server;

void send_RRQ(int sock, char* filepath) {
    int opcode = htons(READ);
    char buffer[BUFFER_SIZE];
    int buffer_len = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    strcpy(buffer+buffer_len, filepath);
    buffer_len += strlen(filepath);

    memcpy(buffer+buffer_len, 0, 1);
    buffer_len++;

    strcpy(buffer+buffer_len, MODE);
    buffer_len += strlen(MODE);

    memcpy(buffer+buffer_len, 0, 1);
    buffer_len++;

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&server, sizeof(server)) < 0) {
        exit(2);
    }
}

void send_WRQ(int sock, char* filepath) {
    int opcode = htons(WRITE);
    char buffer[BUFFER_SIZE];
    int buffer_len = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    strcpy(buffer+buffer_len, filepath);
    buffer_len += strlen(filepath);

    memcpy(buffer+buffer_len, 0, 1);
    buffer_len++;

    strcpy(buffer+buffer_len, MODE);
    buffer_len += strlen(MODE);

    memcpy(buffer+buffer_len, 0, 1);
    buffer_len++;

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&server, sizeof(server)) < 0) {
        exit(2);
    }

}

void send_DATA(int sock, uint16_t packet_number, char* data) {
    int opcode = htons(WRITE);
    char buffer[BUFFER_SIZE];
    int buffer_len = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    packet_number = htons(packet_number);
    memcpy(buffer+buffer_len, &packet_number, 2);
    buffer_len += 2;

    strcpy(buffer+buffer_len, data);

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&server, sizeof(server)) < 0) {
        exit(2);
    }
}

void send_ACK(int sock, uint16_t packet_number) {
    int opcode = htons(ACK);
    char buffer[BUFFER_SIZE];
    int buffer_len = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    packet_number = htons(packet_number);
    memcpy(buffer+buffer_len, &packet_number, 2);
    buffer_len += 2;

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&server, sizeof(server)) < 0) {
        exit(2);
    }
}

void send_ERR(int sock, uint16_t error_code, char* error_msg) {
    int opcode = htons(ERROR);
    char buffer[BUFFER_SIZE];
    int buffer_len = 0;
	error_code = htons(error_code);

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    memcpy(buffer+buffer_len, &error_code, 2);
    buffer_len += 2;

    strcpy(buffer+buffer_len, error_msg);
    buffer_len += strlen(error_msg);

    memcpy(buffer+buffer_len, 0, 1);

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&server, sizeof(server)) < 0) {
        exit(2);
    }
}

int main(int argc, char **argv) {

    char* hostname = "";
    int port = STANDART_PORT;
    char* filepath = "";
    char* dest_filepath = "";
    int operation = WRITE;
    int c;

    while ((c = getopt (argc, argv, ":h:p:f:t:")) != -1) {
        switch (c) {
            case 'h':
                hostname = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'f':
                filepath = optarg;
                operation = READ;
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

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    inet_pton(AF_INET, hostname, &server.sin_addr);
    server.sin_port = htons(port);

    // RRQ/WRQ
    if (operation == WRITE) {
        scanf("%s", filepath);
        send_WRQ(sock, filepath);
    } else if (operation == READ) {
        send_RRQ(sock, filepath);
    }

    // responsible
    if (operation == READ) {
            int opcode;
            char buffer[BUFFER_SIZE];
            int addr_len = sizeof(server);
            int recv_len = recvfrom(sock, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);

            memcpy(&opcode, (uint16_t *) & buffer, 2);
            opcode = ntohs(opcode);

            if (opcode == ERROR) {
                fprintf(stderr, "OPCODE ERROR");
            } else if (opcode == DATA) {
                uint16_t packet_number;
                memcpy(&packet_number, (uint16_t *) & buffer[2], 2);
                packet_number = ntohs(packet_number);

                FILE *dest_file = fopen(dest_filepath, "w");

                if (dest_file == NULL) {
                    exit(-1);
                }

                for (int i = 0; i < recv_len - 4; i++)
                    fputc(buffer[i + 4], dest_file);

                send_ACK(sock, packet_number);

                while (recv_len == 516) {
                    int recv_len = recvfrom(sock, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);
                    
                    memcpy(&packet_number, (uint16_t *) & buffer[2], 2);
                    packet_number = ntohs(packet_number);

                    for (int i = 0; i < recv_len - 4; i++)
                        fputc(buffer[i + 4], dest_file);

                    send_ACK(sock, packet_number);
                }

                shutdown(sock, SHUT_RDWR);
                close(sock);
                fclose(dest_file);            
            }

    } else if (operation == WRITE) {
        int opcode;
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        int addr_len = sizeof(server);
        recvfrom(sock, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);

        memcpy(&opcode, (uint16_t *) & buffer, 2);
        opcode = ntohs(opcode);

        if (opcode == ERROR) {
            fprintf(stderr, "OPCODE ERROR");
        } else if (opcode == ACK) {
            uint16_t packet_number = 1;
            packet_number = ntohs(packet_number);

            FILE *dest_file = fopen(dest_filepath, "r");

            if (dest_file == NULL) {
                exit(-1);
            }

            do {

            } while ()

            shutdown(sock, SHUT_RDWR);
            close(sock);
            fclose(dest_file);  
                
        }            
    }

    return 0; 
}