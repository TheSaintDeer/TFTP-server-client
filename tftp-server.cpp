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
    char* root_dirpath = argv[argc-1];

    int c;

    while ((c = getopt (argc, argv, "p:")) != -1) {
        switch (c) {
            case 'p':
                port = atoi(optarg);
                break;
        }
    }

    //creating socket

    int sock;
    struct sockaddr_in server;

    if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(sock, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }

    fprintf(stdout, "TFTP Server successfully started.\n");

    char addr[17];
    inet_ntop(server.sin_family, (void*) &server.sin_addr, addr, sizeof(addr));

    fprintf(stdout, "Server IP: %s\n", addr);
    fprintf(stdout, "Server Port: %d\n", ntohs(server.sin_port));

    // listing for packets

    struct sockaddr_in client;
    size_t client_size = sizeof(client);
    int recv_len;
    char buffer[PACKET_SIZE];
    uint16_t opcode;
    pid_t fork_id;
    char filename[512];
    char mode[10];

    while(1) {
        recv_len = recvfrom(sock, (char*) buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &client_size);
        
        memcpy(&opcode, (uint16_t*) &buffer, 2);
        opcode = ntohs(opcode);
        fprintf(stdout, "opcode: %d ", opcode);

        strcpy(filename, buffer+2);
        fprintf(stdout, "filename: %s ", filename);

        strcpy(mode, buffer+3+strlen(filename));
        fprintf(stdout, "mode: %s\n", mode);

        if (opcode == READ) {

            if (access(filename, F_OK) == -1) {
                exit(1);
            } else {
                fprintf(stdout, "File exists\n");
            }

            fork_id = fork();

            if (fork_id > 0) {
                memset(buffer, 0, PACKET_SIZE);
                fprintf(stdout, "Parent: go away\n");
            } else if (fork_id == 0) {
                fprintf(stdout, "Child: cont.\n");

                char data[DATA_SIZE];
                memset(data, 0, DATA_SIZE);
                int data_sock = socket(AF_INET, SOCK_DGRAM, 0);
                FILE *src_file;
                fprintf(stdout, "Choosing mode\n");


                if (strncmp(mode, "octet", 5) == 0) {

                    src_file = fopen(filename, "rb");
                    fprintf(stdout, "File opened\n");
                    uint16_t packet_number = 1;
                    uint16_t recv_packet_number;
                    int i = 0;
                    size_t dim = 0;

                    do {
                        
                        dim = fread(&data[i], 1, 1, src_file);
                        i += dim;

                        if (i == DATA_SIZE || dim == 0) {
                            send_DATA(data_sock, packet_number, data, client);
                            fprintf(stdout, "DATA sended i: %d dim: %ld\n", i, dim);
                            memset(data, 0, DATA_SIZE);
                            memset(buffer, 0, PACKET_SIZE);
                            recv_len = recvfrom(data_sock, (char*) buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &client_size);

                            memcpy(&opcode, (uint16_t*) &buffer, 2);
                            opcode = ntohs(opcode);

                            if (opcode != ACK) {
                                fclose(src_file);
				                shutdown(data_sock, SHUT_RDWR);
				                close(data_sock);
                                exit(5);
                            }

                            memcpy(&recv_packet_number, (uint16_t*) &buffer[2], 2);
                            recv_packet_number = ntohs(recv_packet_number);
                            
                            if (packet_number != recv_packet_number) {
                                fclose(src_file);
                                shutdown(data_sock, SHUT_RDWR);
                                close(data_sock);
                                exit(6);
                            }

                            i = 0;
                            packet_number++;
                        }
                    } while (dim == 1);
                    fprintf(stdout, "Ended loop\n");
                    return 0;
                    
                } else if (strncmp(mode, "netascii", 8) == 0) {
                    // TODO mode netascii
                }

            } else {
                exit(2); //fork() error
            }

        }

    }




    // while(1) {

        // if (opcode == READ) {

            // char* path = (char*) malloc(strlen(root_dirpath) + strlen(filename) + 1);

            // // setup requested file full path
            // strcpy(path, root_dirpath);
            // strcat(path, "/");
            // strcat(path, filename);

            // if (access(path, F_OK) == -1) {
            //     exit(1); //file doesn't exist
            // }

            // fork_id = fork();

            // if (fork_id > 0) {
            //     memset(buffer, 0, PACKET_SIZE);
            // } else if (fork_id == 0) {

            //     memset(buffer, 0, PACKET_SIZE);
            //     int data_sock = socket(AF_INET, SOCK_DGRAM, 0);
            //     FILE *src_file;

            //     if (strncpy(mode, "octet", 5) == 0) {
            //         //TODO
            //     } else if (strncpy(mode, "netascii", 8) == 0) {
            //         // TODO
            //     }

            // } else {
            //     exit(2); //fork() error
            // }

        // } else if (opcode == WRITE) {

        // } else {
        //     send_ERR(listener, 101, "Change", client);
        // }
    
    // }
    return 0;
}