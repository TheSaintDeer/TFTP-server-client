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

int main(int argc, char **argv) {

    char* hostname;
    int port = STANDART_PORT;
    char* filepath;
    char* dest_filepath;
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
    struct sockaddr_in server;

    server.sin_family = AF_INET;
    inet_pton(AF_INET, hostname, &server.sin_addr);
    server.sin_port = htons(port);

    // RRQ/WRQ
    if (operation == WRITE) {
        scanf("%s", filepath);
        send_WRQ(sock, filepath, server);
    } else if (operation == READ) {
        send_RRQ(sock, filepath, server);
        fprintf(stdout, "Sended RRQ\n");
    }

    // responsible
    if (operation == READ) {
        int opcode;
        char buffer[PACKET_SIZE];
        int addr_len = sizeof(server);
        int recv_len = recvfrom(sock, (char *)buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);

        memcpy(&opcode, (uint16_t *) & buffer, 2);
        opcode = ntohs(opcode);
        fprintf(stdout, "Opcode: %d\n", opcode);

        if (opcode == ERROR) {
            fprintf(stderr, "OPCODE ERROR");
            exit(10);
        } else if (opcode == DATA) {
            fprintf(stdout, "Have given DATA\n");
            uint16_t packet_number;
            memcpy(&packet_number, (uint16_t *) & buffer[2], 2);
            packet_number = ntohs(packet_number);

            fprintf(stdout, "filepath: %s\n", dest_filepath);
            FILE *dest_file = fopen(dest_filepath, "w+b");

            if (dest_file == NULL) {
                exit(-1);
            }

            char data[DATA_SIZE];
            strcpy(data, buffer+4);

            fprintf(stdout, "opcode: %d\npacket number: %d\ndata: %s\n", opcode, packet_number, data);

            for (int i = 4; i < recv_len; i++)
                fputc(buffer[i], dest_file);

            send_ACK(sock, packet_number, server);
            fprintf(stdout, "Sended ACK\n");

            while (recv_len == 516) {
                fprintf(stdout, "Started loop\n");
                int recv_len = recvfrom(sock, (char *)buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);
                
                memcpy(&packet_number, (uint16_t *) & buffer[2], 2);
                packet_number = ntohs(packet_number);

                for (int i = 4; i < recv_len; i++)
                    fputc(buffer[i], dest_file);

                send_ACK(sock, packet_number, server);
                fprintf(stdout, "Sended ACK\n");
            }

            shutdown(sock, SHUT_RDWR);
            close(sock);
            fclose(dest_file);            
        }
    }
    // } else if (operation == WRITE) {    

    //     char buffer[PACKET_SIZE];
    //     memset(buffer, 0, PACKET_SIZE);
    //     char data[DATA_SIZE];
    //     memset(data, 0, DATA_SIZE);
    //     uint16_t opcode;
    //     int addr_len = sizeof(server);
    //     char c;
    //     int i = 0;
    //     int packet_number = 0;
    //     int recv_packet_number;

    //     FILE *dest_file = fopen(dest_filepath, "r");

    //     if (dest_file == NULL) {
    //         exit(-1);
    //     }

    //     int recv_len = recvfrom(sock, (char *)buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);

    //     memcpy(&opcode, (uint16_t *) & buffer, 2);
    //     opcode = ntohs(opcode);

    //     if (opcode == ERROR) {
    //         fprintf(stderr, "OPCODE ERROR");
    //         exit(10);
    //     } else if (opcode == ACK) {
        
    //         memcpy(&recv_packet_number, (uint16_t *) & buffer[2], 2);
    //         if (packet_number != ntohs(recv_packet_number))
    //             exit(11);

    //         do {
    //             c = fgetc(dest_file);

    //             if (c != EOF) {
    //                 data[i] = c;
    //                 i++;
    //             } 
                
    //             if (c == EOF || i == DATA_SIZE) {
    //                 packet_number++;
    //                 send_DATA(sock, packet_number, data, server);

    //                 recv_len = recvfrom(sock, (char *)buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);

    //                 memcpy(&opcode, (uint16_t *) & buffer, 2);
    //                 opcode = ntohs(opcode);
    //                 if (opcode == ERROR) {
    //                     fprintf(stderr, "OPCODE ERROR");
    //                     exit(10);
    //                 } else if (opcode == ACK) {
    //                     memcpy(&recv_packet_number, (uint16_t *) & buffer[2], 2);
    //                     if (packet_number != ntohs(recv_packet_number))
    //                         exit(11);
    //                 }

    //                 i = 0;
    //                 memset(data, 0, DATA_SIZE);

    //             }

    //         } while (c != EOF);

    //         shutdown(sock, SHUT_RDWR);
    //         close(sock);
    //         fclose(dest_file); 
    //     }

    // }

    return 0; 
}