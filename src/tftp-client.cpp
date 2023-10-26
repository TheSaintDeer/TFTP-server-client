#include "../include/tftp-client.hpp"

void get_parametrs(struct parametrs* p, int* oper, int argc, char **argv) {
    int c;

    while ((c = getopt (argc, argv, ":h:p:f:t:")) != -1) {
        switch (c) {
            case 'h':
                p->hostname = optarg;
                break;
            case 'p':
                p->port = atoi(optarg);
                break;
            case 'f':
                p->filepath = optarg;
                *oper = RRQ;
                break;
            case 't':
                p->dest_filepath = optarg;
                break;
            case '?':
                if (optopt == 'h' || optopt == 't') 
                    fprintf(stderr, "Option -%c requires an argument\n", optopt);
                else
                    fprintf(stderr, "Unknown option\n");
	  	        
	            exit(1);
        }
    }
}

void get_respones_RRQ(int sock, sockaddr_in server, struct parametrs p) {
    int opcode;
    char buffer[PACKET_SIZE];
    int addr_len = sizeof(server);
    int recv_len = recvfrom(sock, (char *)buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);

    memcpy(&opcode, (uint16_t *) &buffer, 2);
    opcode = ntohs(opcode);
    fprintf(stdout, "log: received packet with opcode %d.\n", opcode);

    if (opcode == ERROR) {
        char msg[DATA_SIZE];
        strcpy(msg, buffer+4);
        fprintf(stderr, "log: error - %s.\n", msg);
        exit(1);
    } else if (opcode == DATA) {
        uint16_t packet_number;
        memcpy(&packet_number, (uint16_t *) & buffer[2], 2);
        packet_number = ntohs(packet_number);

        FILE *dest_file;

        if ((dest_file = fopen(p.dest_filepath, "w+b")) == NULL) {
            fprintf(stdout, "log: could not open file.\n");
            exit(-1);
        }

        char data[DATA_SIZE];
        strcpy(data, buffer+4);
        fprintf(stdout, "log: received data: %s.\n", data);

        for (int i = 4; i < recv_len; i++)
            fputc(buffer[i], dest_file);

        send_ACK(sock, packet_number, server);
        fprintf(stdout, "log: sended ACK packet.\n");

        while (recv_len == 516) {
            int recv_len = recvfrom(sock, (char *)buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);
            
            memcpy(&opcode, (uint16_t *) & buffer, 2);
            opcode = ntohs(opcode);
            fprintf(stdout, "log: received packet with opcode %d.\n", opcode);

            if (opcode == DATA) {
                memcpy(&packet_number, (uint16_t *) & buffer[2], 2);
                packet_number = ntohs(packet_number);

                strcpy(data, buffer+4);
                fprintf(stdout, "log: received data: %s.\n", data);

                for (int i = 4; i < recv_len; i++)
                    fputc(buffer[i], dest_file);

                send_ACK(sock, packet_number, server);
                fprintf(stdout, "log: sended ACK packet.\n");
            } else if (opcode == ERROR) {
                char msg[DATA_SIZE];
                strcpy(msg, buffer+4);
                fprintf(stderr, "log: error - %s.\n", msg);
                exit(1);
            }

        }

        shutdown(sock, SHUT_RDWR);
        close(sock);
        fclose(dest_file);      
    }
}

void get_respones_WRQ(int sock, sockaddr_in server, struct parametrs p) {
    int opcode;
    char buffer[PACKET_SIZE];
    int addr_len = sizeof(server);
    int recv_len = recvfrom(sock, (char *)buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);

    memcpy(&opcode, (uint16_t *) &buffer, 2);
    opcode = ntohs(opcode);
    fprintf(stdout, "log: received packet with opcode %d.\n", opcode);

    if (opcode == ERROR) {
        char msg[DATA_SIZE];
        strcpy(msg, buffer+4);
        fprintf(stderr, "log: error - %s.\n", msg);
        exit(1);
    } else if (opcode == ACK) {
        char data[DATA_SIZE];
        uint16_t packet_number;
        uint16_t recv_packet_number;
        int i = 0;
        size_t dim = 0;
        
        memcpy(&recv_packet_number, (uint16_t *) &buffer[2], 2);
        recv_packet_number = ntohs(recv_packet_number);
        packet_number = recv_packet_number;

        FILE *src_file = fopen(p.filepath, "rb");

        memset(buffer, 0, PACKET_SIZE);
        memset(data, 0, DATA_SIZE);

        do {
            dim = fread(&data[i], 1, 1, src_file);
            i += dim;

            if (i == DATA_SIZE || dim == 0) {
                send_DATA(sock, packet_number, data, server);
                fprintf(stdout, "log: DATA was sended\n");
                memset(data, 0, DATA_SIZE);
                memset(buffer, 0, PACKET_SIZE);
                recv_len = recvfrom(sock, (char*) buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr*) &server, (socklen_t*) &addr_len);

                memcpy(&opcode, (uint16_t*) &buffer, 2);
                opcode = ntohs(opcode);

                if (opcode != ACK) {
                    fclose(src_file);
                    shutdown(sock, SHUT_RDWR);
                    close(sock);
                    exit(1);
                }

                memcpy(&recv_packet_number, (uint16_t*) &buffer[2], 2);
                recv_packet_number = ntohs(recv_packet_number);
                
                if (packet_number != recv_packet_number) {
                    fclose(src_file);
                    shutdown(sock, SHUT_RDWR);
                    close(sock);
                    exit(1);
                }

                i = 0;
                packet_number++;
            }
        } while(dim == 1);        
    }
}

int main(int argc, char **argv) {

    struct parametrs p;
    int operation = WRQ;

    get_parametrs(&p, &operation, argc, argv);

    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        fprintf(stderr, "log: socket creation failed.\n");
        exit(EXIT_FAILURE); 
    } 
    fprintf(stderr, "log: socket created.\n");
    struct sockaddr_in server;

    server.sin_family = AF_INET;
    inet_pton(AF_INET, p.hostname, &server.sin_addr);
    server.sin_port = htons(p.port);

    // RRQ/WRQ
    if (operation == WRQ) {
        scanf("%s", p.filepath);
        send_WRQ(sock, p.dest_filepath, server);
        fprintf(stdout, "log: sended WRQ packet.\n");
        get_respones_WRQ(sock, server, p);
    } else if (operation == RRQ) {
        send_RRQ(sock, p.filepath, server);
        fprintf(stdout, "log: sended RRQ packet.\n");
        get_respones_RRQ(sock, server, p);
    }

    return 0; 
}