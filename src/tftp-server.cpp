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

int control_OACK(struct opts* o, char* buffer, int buffer_len, int recv_len) {
    char option[512];
    char value[512];
    char result[PACKET_SIZE];


    if (buffer_len == recv_len) {
        strcpy(o->blksize, "512");
        return 2;
    }
    

    while (buffer_len < recv_len) {
        strcpy(option, buffer+buffer_len);
        buffer_len += strlen(option) + 1;
        
        if (strcmp(option, "blksize") == 0) {
            strcpy(value, buffer+buffer_len);
            buffer_len += strlen(value) + 1;
            char number[3];
            for (int i = 0; i < strlen(value) - 1; i += 2){
                memset(number, '\0', sizeof(number));
                strncpy(number, value+i, 2);
                result[i/2] = convert_from_ASCII(number);
            }
            strcpy(o->blksize, result);

        } else if (strcmp(option, "timeout") == 0) {
            strcpy(value, buffer+buffer_len);
            buffer_len += strlen(value) + 1;
            char number[3];
            for (int i = 0; i < strlen(value) - 1; i += 2){
                memset(number, '\0', sizeof(number));
                strncpy(number, value+i, 2);
                result[i/2] = convert_from_ASCII(number);
            }
            strcpy(o->timeout, result);
        } else if (strcmp(option, "tsize") == 0) {
            strcpy(value, buffer+buffer_len);
            buffer_len += strlen(value) + 1;
            char number[3];
            for (int i = 0; i < strlen(value) - 1; i += 2){
                memset(number, '\0', sizeof(number));
                strncpy(number, value+i, 2);
                result[i/2] = convert_from_ASCII(number);
            }
            strcpy(o->tsize, result);
        } else {
            return 1;
        }
        memset(result, '\0', PACKET_SIZE);
    }

    return 0;
}

void main_loop(struct parametrs p) {

    // creating socket
    int sock;
    struct sockaddr_in server, client;
    size_t addr_len = sizeof(client);

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

    while(1) {
        recv_len = recvfrom(sock, (char*) buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &addr_len);

        memcpy(&opcode, (uint16_t*) &buffer, 2);
        opcode = ntohs(opcode);
         
        if (opcode == RRQ) {
            proccessing_RRQ(sock, client, addr_len, buffer, recv_len);
        } else if (opcode == WRQ) {
            proccessing_WRQ(sock, client, addr_len, buffer, recv_len);
        }
    }
}

void proccessing_RRQ(int sock, sockaddr_in client, size_t addr_len, char* buffer, int recv_len) {
    struct opts o;
    char filename[512];
    int buffer_len = 2;
    char mode[10];

    strcpy(filename, buffer+buffer_len);
    buffer_len += strlen(filename) + 1;

    strcpy(mode, buffer+buffer_len);
    buffer_len += strlen(mode) + 1;

    pid_t fork_id = fork();

    if (fork_id > 0) { //parent
        return;
    } else if (fork_id == 0) { //children
        FILE *src_file;
        if (strncmp(mode, "octet", 5) == 0) {
            src_file = fopen(filename, "rb");
        } else if (strncmp(mode, "netascii", 8) == 0) {
            src_file = fopen(filename, "r");
        }

        if (src_file == NULL) {
            send_ERR(sock, 1, client);
            return;
        }

        int child_sock = socket(AF_INET, SOCK_DGRAM, 0);

        int res = control_OACK(&o, buffer, buffer_len, recv_len);
        if (res == 2) {

        } else if (res == 1)
            send_ERR(child_sock, 8, client);
        else if (res == 0) {
            if (strcmp(o.timeout, "-1") != 0) {
                struct timeval timeout;
                timeout.tv_sec = stoi(o.timeout);
                timeout.tv_usec = 0;

                if (setsockopt(child_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
                    send_ERR(sock, 0, client);
                    return;
                }
            }

            if (strcmp(o.tsize, "0") == 0) { 
                fseek(src_file, 0, SEEK_END);
                int file_size = ftell(src_file);
                fseek(src_file, 0, SEEK_SET);

                sprintf(o.tsize, "%d", file_size); 
            }

            send_OACK(child_sock, o, client);
        }

        if (strncmp(mode, "octet", 5) == 0) {
            RRQ_octet(child_sock, client, addr_len, o, src_file, res);
        } else if (strncmp(mode, "netascii", 8) == 0) {
            RRQ_netascii(child_sock, client, addr_len, o, src_file, res);
        } else {
            return;
        }

    } else if (fork_id < 0) {
        send_ERR(sock, 0, client);
        fprintf(stderr, "log: error while creating child process for client.\n");
        return;
    }   
}

void RRQ_octet(int sock, sockaddr_in client, size_t client_size, struct opts o, FILE* src_file, bool with_opts) {

    int blksize = stoi(o.blksize);
    char buffer[blksize+4];
    memset(buffer, 0, blksize+4);
    int recv_len;
    int opcode;
    
    uint16_t packet_number = 1;
    uint16_t recv_packet_number;

    if (!with_opts) {

        recv_len = recvfrom(sock, (char*) buffer, blksize+4, MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &client_size);
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
        
        if (0 != recv_packet_number) {
            fclose(src_file);
            shutdown(sock, SHUT_RDWR);
            close(sock);
            exit(1);
        }
    }

    char data[blksize];
    size_t dim = 0;
    int i = 0;
    do {
        dim = fread(&data[i], 1, 1, src_file);
        i += dim;

        if (i == blksize || dim == 0) {
            send_DATA(sock, packet_number, data, blksize+4, client);
            memset(data, 0, blksize);
            memset(buffer, 0, blksize+4);
            recv_len = recvfrom(sock, (char*) buffer, blksize+4, MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &client_size);

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

void RRQ_netascii(int sock, sockaddr_in client, size_t client_size, struct opts o, FILE* src_file, bool with_opts) {

    int blksize = PACKET_SIZE-4;
    if (strcmp(o.blksize, "-1") != 0) {
        blksize = stoi(o.blksize);
    }

    char buffer[blksize+4];
    int recv_len;
    int opcode;

    memset(buffer, 0, blksize+4);
    
    uint16_t packet_number = 1;
    uint16_t recv_packet_number;

    if (!with_opts) {
        recv_len = recvfrom(sock, (char*) buffer, blksize+4, MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &client_size);
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
        
        if (0 != recv_packet_number) {
            fclose(src_file);
            shutdown(sock, SHUT_RDWR);
            close(sock);
            exit(1);
        }
    }


    char data[blksize];
    int i = 0;
    char c;

    memset(buffer, 0, blksize+4);
    memset(data, 0, blksize);

    do {
        c = fgetc(src_file);

        if(c != EOF) {
            i++;
            data[i] = c;
        }

        if (i == blksize || c == EOF) {
            send_DATA(sock, packet_number, data, blksize+4, client);
            memset(data, 0, blksize);
            memset(buffer, 0, blksize+4);
            recv_len = recvfrom(sock, (char*) buffer, blksize+4, MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &client_size);

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

void proccessing_WRQ(int sock, sockaddr_in client, size_t addr_len, char* buffer, int recv_len) {
    struct opts o;
    char filename[512];
    int buffer_len = 2;
    char mode[10];

    strcpy(filename, buffer+buffer_len);
    buffer_len += strlen(filename) + 1;

    strcpy(mode, buffer+buffer_len);

    pid_t fork_id = fork();

    if (fork_id > 0) { //parent
        return;
    } else if (fork_id == 0) { //children
        FILE *src_file;
        if (strncmp(mode, "octet", 5) == 0) {
            src_file = fopen(filename, "wb");
        } else if (strncmp(mode, "netascii", 8) == 0) {
            src_file = fopen(filename, "w");
        }

        if (src_file == NULL) {
            send_ERR(sock, 6, client);
            return;
        }

        int child_sock = socket(AF_INET, SOCK_DGRAM, 0);

        int res = control_OACK(&o, buffer, buffer_len, recv_len);
        if (res == 2) {
            send_ACK(sock, 0, client);
        } else if (res == 1)
            send_ERR(sock, 8, client);
        else if (res == 0) {
            if (strcmp(o.timeout, "-1") != 0) {
                struct timeval timeout;
                timeout.tv_sec = stoi(o.timeout);
                timeout.tv_usec = 0;

                if (setsockopt(child_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
                    send_ERR(sock, 0, client);
                    return;
                }
            }

            if (strcmp(o.tsize, "-1") != 0) { 
                long pages = sysconf(_SC_PHYS_PAGES);
                long page_size = sysconf(_SC_PAGE_SIZE);
                if (pages*page_size < stoi(o.tsize)) {
                    send_ERR(sock, 3, client);
                    return;
                }
            }

            send_OACK(sock, o, client);
        }

        WRQ_communication(child_sock, client, addr_len, o, src_file, res);

    } else if (fork_id < 0) {
        send_ERR(sock, 0, client);
        return;
    }
}

void WRQ_communication(int sock, sockaddr_in client, size_t addr_len, struct opts o, FILE* src_file, bool with_opts) {
    
    int blksize = PACKET_SIZE-4;
    if (strcmp(o.blksize, "-1") != 0) {
        blksize = stoi(o.blksize);
    }

    int opcode;
    char buffer[PACKET_SIZE];
    int recv_len = blksize+4;
    uint16_t packet_number;

    while (recv_len == blksize+4) {
        int recv_len = recvfrom(sock, (char *)buffer, blksize+4, MSG_WAITALL, (struct sockaddr *)&client, (socklen_t *) &addr_len);
        
        memcpy(&opcode, (uint16_t *) &buffer, 2);
        opcode = ntohs(opcode);

        if (opcode == DATA) {
            memcpy(&packet_number, (uint16_t *) & buffer[2], 2);
            packet_number = ntohs(packet_number);

            for (int i = 4; i < recv_len; i++)
                fputc(buffer[i], src_file);

            send_ACK(sock, packet_number, client);
            memset(buffer, 0, blksize+4);
        } else if (opcode == ERROR) {
            exit(1);
        }
    }
    shutdown(sock, SHUT_RDWR);
    close(sock);
    fclose(src_file);
}

int main(int argc, char **argv) {

    struct parametrs p;
    get_parametrs(&p, argc, argv);

    main_loop(p);

    return 0;
}