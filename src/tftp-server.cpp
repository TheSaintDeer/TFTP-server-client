#include "../include/tftp-server.hpp"

/**
 * Accept and process input parameters to start the client
*/
void get_parametrs(struct parametrs* p, int argc, char **argv) {
    int c;

    while ((c = getopt (argc, argv, "p:")) != -1) {
        switch (c) {
            case 'p':
                p->port = atoi(optarg);
                break;
        }
    }

    p->root_dirpath = argv[argc-1];
}

void main_loop(struct parametrs p) {

    // creating socket
    int sock;
    struct sockaddr_in server, client;
    size_t client_size = sizeof(client);

    if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        fprintf(stderr, "log: socket creation failed.\n"); 
        exit(EXIT_FAILURE); 
    } 
    fprintf(stdout, "log: socket created.\n"); 

    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(p.port);

    if (bind(sock, (struct sockaddr*) &server, sizeof(server)) < 0) {
        fprintf(stderr, "log: bind failed\n"); 
        exit(EXIT_FAILURE); 
    }
    fprintf(stderr, "log: bind success.\n"); 

    char addr[17];
    inet_ntop(server.sin_family, (void*) &server.sin_addr, addr, sizeof(addr));

    // listing for packets

    int recv_len;
    char buffer[PACKET_SIZE];
    uint16_t opcode;
    char filename[512];
    char mode[10];

    while(1) {
        recv_len = recvfrom(sock, (char*) buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &client_size);

        memcpy(&opcode, (uint16_t*) &buffer, 2);
        opcode = ntohs(opcode);

        strcpy(filename, buffer+2);
        strcpy(mode, buffer+3+strlen(filename));
        fprintf(stdout, "log: received packet with opcode: %d filename: %s mode: %s\n", opcode, filename, mode);
        memset(buffer, 0, PACKET_SIZE);

        if (opcode == RRQ) {
            proccessing_RRQ(sock, client, filename, mode, client_size);
        } else if (opcode == WRQ) {
            proccessing_WRQ(sock, client, filename, mode, client_size);
        }

    }
}

void proccessing_RRQ(int sock, sockaddr_in client, char* filename, char* mode, size_t client_size) {

    if (access(filename, F_OK) == -1)
        send_ERR(sock, FILE_NOT_FOUND, client);

    pid_t fork_id = fork();

    if (fork_id > 0) { //parent
        
    } else if (fork_id == 0) { //children

        int child_sock = socket(AF_INET, SOCK_DGRAM, 0);

        if (strncmp(mode, "octet", 5) == 0) {
            RRQ_octet(child_sock, filename, client, client_size);
        } else if (strncmp(mode, "netascii", 8) == 0) {
            RRQ_netascii(child_sock, filename, client, client_size);
        } else {
            exit(2);
        }

    } else if (fork_id < 0) {
        fprintf(stderr, "log: error while creating child process for client.\n");
        exit(1);
    }   
}

void RRQ_octet(int sock, char* filename, sockaddr_in client, size_t client_size) {
    FILE *src_file = fopen(filename, "rb");
    char buffer[PACKET_SIZE];
    char data[DATA_SIZE];
    int recv_len;
    int opcode;
    uint16_t packet_number = 1;
    uint16_t recv_packet_number;
    int i = 0;
    size_t dim = 0;

    memset(buffer, 0, PACKET_SIZE);
    memset(data, 0, DATA_SIZE);

    do {
        dim = fread(&data[i], 1, 1, src_file);
        i += dim;

        if (i == DATA_SIZE || dim == 0) {
            send_DATA(sock, packet_number, data, client);
            fprintf(stdout, "log: DATA was sended\n");
            memset(data, 0, DATA_SIZE);
            memset(buffer, 0, PACKET_SIZE);
            recv_len = recvfrom(sock, (char*) buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &client_size);

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
    } while (dim == 1);
}

void RRQ_netascii(int sock, char* filename, sockaddr_in client, size_t client_size) {
    FILE *src_file = fopen(filename, "r");
    char buffer[PACKET_SIZE];
    char data[DATA_SIZE];
    int recv_len;
    int opcode;
    uint16_t packet_number = 1;
    uint16_t recv_packet_number;
    int i = 0;
    char c;

    memset(buffer, 0, PACKET_SIZE);
    memset(data, 0, DATA_SIZE);

    do {
        
        c = fgetc(src_file);

        if(c != EOF) {
            i++;
            data[i] = c;
        }

        if (i == DATA_SIZE || c == EOF) {
            send_DATA(sock, packet_number, data, client);
            fprintf(stdout, "log: DATA was sended\n");
            memset(data, 0, DATA_SIZE);
            memset(buffer, 0, PACKET_SIZE);
            recv_len = recvfrom(sock, (char*) buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &client_size);

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
    } while (c != EOF);
}

void proccessing_WRQ(int sock, sockaddr_in client, char* filename, char* mode, size_t client_size) {

    pid_t fork_id = fork();

    if (fork_id > 0) { //parent
        
    } else if (fork_id == 0) { //children

        int child_sock = socket(AF_INET, SOCK_DGRAM, 0);

        if (strncmp(mode, "octet", 5) == 0) {
            WRQ_octet(child_sock, filename, client, client_size);
        } else if (strncmp(mode, "netascii", 8) == 0) {
            WRQ_netascii(child_sock, filename, client, client_size);
        } else {
            exit(2);
        }

    } else if (fork_id < 0) {
        fprintf(stderr, "log: error while creating child process for client.\n");
        exit(1);
    }   
}

void WRQ_octet(int sock, char* filename, sockaddr_in client, size_t client_size) {
    int opcode;
    char buffer[PACKET_SIZE];
    int recv_len = recvfrom(sock, (char *)buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&client, (socklen_t *) &client_size);

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

        if ((dest_file = fopen(filename, "w+b")) == NULL) {
            fprintf(stdout, "log: could not open file.\n");
            exit(-1);
        }

        char data[DATA_SIZE];
        strcpy(data, buffer+4);
        fprintf(stdout, "log: received data: %s.\n", data);

        for (int i = 4; i < recv_len; i++)
            fputc(buffer[i], dest_file);

        send_ACK(sock, packet_number, client);
        fprintf(stdout, "log: sended ACK packet.\n");

        while (recv_len == 516) {
            int recv_len = recvfrom(sock, (char *)buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&client, (socklen_t *) & client_size);
            
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

                send_ACK(sock, packet_number, client);
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

void WRQ_netascii(int sock, char* filename, sockaddr_in client, size_t client_size) {
    int opcode;
    char buffer[PACKET_SIZE];
    int recv_len = recvfrom(sock, (char *)buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&client, (socklen_t *) &client_size);

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

        if ((dest_file = fopen(filename, "w+")) == NULL) {
            fprintf(stdout, "log: could not open file.\n");
            exit(-1);
        }

        char data[DATA_SIZE];
        strcpy(data, buffer+4);
        fprintf(stdout, "log: received data: %s.\n", data);

        for (int i = 4; i < recv_len; i++)
            fputc(buffer[i], dest_file);

        send_ACK(sock, packet_number, client);
        fprintf(stdout, "log: sended ACK packet.\n");

        while (recv_len == 516) {
            int recv_len = recvfrom(sock, (char *)buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr *)&client, (socklen_t *) & client_size);
            
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

                send_ACK(sock, packet_number, client);
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

int main(int argc, char **argv) {

    struct parametrs p;
    get_parametrs(&p, argc, argv);

    main_loop(p);

    return 0;
}