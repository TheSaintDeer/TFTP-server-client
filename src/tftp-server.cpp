/**
 * Name: Ivan Golikov
 * Login: xgolik00
 * 
 * tftp-server [-p port] root_dirpath
 * -p místní port, na kterém bude server očekávat příchozí spojení
 * root_dirpath - cesta k adresáři, pod kterým se budou ukládat příchozí soubory
*/
#include "../include/tftp-server.hpp"

void main_loop(struct parametrs p) {

    // create socket
    struct sockaddr_in server;
    size_t addr_len = sizeof(client);
    char addr[17];

    if ( (sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        fprintf(stderr, "log: Socket creation failed.\n"); 
        exit(-2); 
    } 

    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(p.port);

    // bind a name to the socket
    if (bind(sock, (struct sockaddr*) &server, sizeof(server)) < 0) {
        fprintf(stderr, "log: Bind failed\n"); 
        exit(-3); 
    }

    inet_ntop(server.sin_family, (void*) &server.sin_addr, addr, sizeof(addr));

    // listing for packets
    int recv_len;
    char buffer[PACKET_SIZE];
    uint16_t opcode;
    pid_t fork_id;

    while(1) {
        recv_len = recvfrom(sock, (char*) buffer, PACKET_SIZE, MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &addr_len);

        memcpy(&opcode, (uint16_t*) &buffer, 2);
        opcode = ntohs(opcode);
        
        if (opcode != WRQ && opcode != RRQ)
            send_ERR(sock, 4, client);

        // create sub-process
        fork_id = fork();

        if (fork_id > 0) { //parent
        } else if (fork_id == 0) { // child
            processing_request(opcode, addr_len, buffer, recv_len, p);
        } else { //error
            send_ERR(sock, 0, client);
            exit(0);
        }

    }
}

void processing_request(int op, size_t addr_len, char* buffer, int recv_len, struct parametrs p) {
    struct opts o;
    char filename[512];
    int buffer_len = 2;
    char mode[10];

    // determining the full path to the file
    char *filepath = p.root_dirpath;   
    strcat(filepath, "/");

    strcpy(filename, buffer+buffer_len);
    buffer_len += strlen(filename) + 1;
    strcat(filepath, filename);

    strcpy(mode, buffer+buffer_len);
    buffer_len += strlen(mode) + 1;

    // standardized mode
    for(int i = 0; mode[i]; i++)
        mode[i] = tolower(mode[i]);

    int child_sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (op == RRQ) 
        fprintf(stdout, "RRQ %s:%d \"%s\" %s", inet_ntoa(client.sin_addr), (int) ntohs(client.sin_port), filename, mode);
    else
        fprintf(stdout, "WRQ %s:%d \"%s\" %s", inet_ntoa(client.sin_addr), (int) ntohs(client.sin_port), filename, mode);

    // work with or without OPTS
    int res = control_opts(&o, buffer, buffer_len, recv_len);
    FILE *file = open_file(op, filepath, mode);
    switch (res) { // unknown OPT
        case 2:
            send_ERR(child_sock, 8, client);
            exit(2);
        case 1: // without OPTS
            if (op == WRQ) 
                send_ACK(child_sock, 0, client);
            break;
        case 0: // with OPTS
            handling_opts(op, child_sock, file, &o);
            send_OACK(child_sock, o, client);
            break;
    }

    op == RRQ ? RRQ_handling(addr_len, o, file, res, mode, child_sock) : WRQ_handling(addr_len, o, file, child_sock);

    fclose(file);
    shutdown(child_sock, SHUT_RDWR);
    close(child_sock);
    // finish child process
    exit(10);
}

void RRQ_handling(size_t addr_len, struct opts o, FILE *file, int without_opts, char *mode, int c_sock) {
    int blksize = stoi(o.blksize);
    int opcode;

    if (!without_opts)
        control_ACK(file, blksize+4, addr_len, 0, c_sock);

    if (strcmp(mode, "octet") == 0)
        RRQ_octet_loop(file, addr_len, blksize, c_sock);
    else
        RRQ_netascii_loop(file, addr_len, blksize, c_sock);
}

void RRQ_octet_loop(FILE *file, size_t addr_len, int blksize, int c_sock) {
    uint16_t packet_number = 1;
    uint16_t recv_packet_number;
    char buffer[blksize+4];
    char data[blksize+1];
    int opcode;
    size_t dim = 0;
    int i = 0;
    memset(data, '\0', blksize+1);
    memset(buffer, '\0', blksize+4);

    // while the file is finished
    do {
        dim = fread(&data[i], 1, 1, file);
        i += dim;

        // if the length of the read data is equal to the block size
        if (i == blksize || dim == 0) {
            send_DATA(c_sock, packet_number, data, blksize+4, client);
            control_ACK(file, blksize+4, addr_len, packet_number, c_sock);

            i = 0;
            packet_number++;
            memset(data, '\0', blksize+1);
            memset(buffer, '\0', blksize+4);
        }
    } while (dim == 1);
}

void RRQ_netascii_loop(FILE *file, size_t addr_len, int blksize, int c_sock) {
    uint16_t packet_number = 1;
    uint16_t recv_packet_number;
    char buffer[blksize+4];
    char data[blksize+1];
    int opcode;
    int i = 0;
    char c;
    memset(data, '\0', blksize+1);
    memset(buffer, '\0', blksize+4);

    // while char is not EOF
    do {
        c = fgetc(file);

        if(c != EOF) {
            data[i] = c;
            i++;
        }

        // if the length of the read data is equal to the block size
        if (i == blksize || c == EOF) {
            send_DATA(c_sock, packet_number, data, blksize+4, client);
            control_ACK(file, blksize+4, addr_len, packet_number, c_sock);

            i = 0;
            packet_number++;
            memset(data, '\0', blksize+1);
            memset(buffer, '\0', blksize+4);
        }
    } while (c != EOF);

}

void WRQ_handling(size_t addr_len, struct opts o, FILE* file, int c_sock) {
    int blksize = stoi(o.blksize);
    int opcode;
    char buffer[blksize+4];
    uint16_t packet_number;
    int recv_len = blksize+4;
    struct sockaddr_in sa;
    int sa_len = sizeof(sa);

    // while the length of the received data is equal to the block size
    while (recv_len == blksize+4) {
        recv_len = recvfrom(c_sock, (char *)buffer, blksize+4, MSG_WAITALL, (struct sockaddr *)&client, (socklen_t *) &addr_len);
        
        memcpy(&opcode, (uint16_t *) &buffer, 2);
        opcode = ntohs(opcode);

        if (opcode == DATA) {
            memcpy(&packet_number, (uint16_t *) & buffer[2], 2);
            packet_number = ntohs(packet_number);

            // write data to the file
            for (int i = 4; i < recv_len; i++)
                fputc(buffer[i], file);

            if (getsockname(c_sock,(struct sockaddr *)&sa,(socklen_t *)&sa_len))
                exit(-2);

            fprintf(stdout, "DATA %s:%d:%d %d\n", inet_ntoa(client.sin_addr), (int) ntohs(client.sin_port), (int) ntohs(sa.sin_port), packet_number);
            send_ACK(c_sock, packet_number, client);
            memset(buffer, 0, blksize+4);
        } else {
            send_ERR(c_sock, 4, client);
            exit(4);
        }
    }
}

FILE *open_file(int op, char *filepath, char *mode) {

    if (op == WRQ) {
        // if the file is already existed 
        if (access(filepath, F_OK) != -1) {
            send_ERR(sock, 6, client);
            exit(6);
        }

        create_path(filepath);

        if (strncmp(mode, "octet", 5) == 0)
            return fopen(filepath, "wb");
        else if (strncmp(mode, "netascii", 8) == 0) 
            return fopen(filepath, "w");

    } else {
        // if the file doesn't exist
        if (access(filepath, F_OK) == -1) {
            send_ERR(sock, 1, client);
            exit(1);
        }

        if (strncmp(mode, "octet", 5) == 0) 
            return fopen(filepath, "rb");
        else if (strncmp(mode, "netascii", 8) == 0)
            return fopen(filepath, "r");
    }

    send_ERR(sock, 4, client);
    exit(4);
}

int control_opts(struct opts* o, char* buffer, int buffer_len, int recv_len) {
    char option[512];
    char value[512];
    char result[512];

    if (buffer_len == recv_len) {
        strcpy(o->blksize, "512");
        fprintf(stdout, "\n");
        return 1;
    }

    while (buffer_len < recv_len) {
        // read option
        memset(option, '\0', 512);
        strcpy(option, buffer+buffer_len);
        buffer_len += strlen(option) + 1;

        // read option's value
        memset(value, '\0', 512);
        strcpy(value, buffer+buffer_len);
        buffer_len += strlen(value) + 1;
        
        // get option and its value
        if (strcmp(option, "blksize") == 0) {
            get_and_convert_ascii(value, o->blksize);
            fprintf(stdout, " blksize=%s", o->blksize);

        } else if (strcmp(option, "timeout") == 0) {
            get_and_convert_ascii(value, o->timeout);
            fprintf(stdout, " timeout=%s", o->timeout);

        } else if (strcmp(option, "tsize") == 0) {
            get_and_convert_ascii(value, o->tsize);
            fprintf(stdout, " tsize=%s", o->tsize);
            
        } else {
            return 2;
        }
    }
    fprintf(stdout, "\n");
    return 0;
}

void handling_opts(int op, int child_sock, FILE *file, struct opts *o) {
    // // compare option and its value
    if (strcmp(o->timeout, "-1") != 0) {
        struct timeval timeout;
        timeout.tv_sec = stoi(o->timeout);
        timeout.tv_usec = 0;

        if (setsockopt(child_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            send_ERR(child_sock, 0, client);
            exit(0);
        }
    }

    // determining file size or checking free memory
    if (strcmp(o->tsize, "-1") != 0 && op == WRQ) { 
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        if (pages*page_size < stoi(o->tsize)) {
            send_ERR(child_sock, 3, client);
            exit(3);
        }
    } else if (strcmp(o->tsize, "0") == 0 && op == RRQ) {
        fseek(file, 0, SEEK_END);
        int file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        sprintf(o->tsize, "%d", file_size); 
    }
}

void control_ACK(FILE *file, int packet_len, size_t addr_len, uint16_t packet_number, int c_sock) {
    int opcode;
    char buffer[packet_len];

    recvfrom(c_sock, buffer, packet_len, MSG_WAITALL, (struct sockaddr*) &client, (socklen_t*) &addr_len);
    memcpy(&opcode, (uint16_t*) &buffer, 2);
    opcode = ntohs(opcode);

    uint16_t recv_packet_number;
    memcpy(&recv_packet_number, (uint16_t*) &buffer[2], 2);
    recv_packet_number = ntohs(recv_packet_number);
    
    if (packet_number != recv_packet_number || opcode != ACK) {
        fclose(file);
        shutdown(c_sock, SHUT_RDWR);
        close(c_sock);
        exit(0);
    }

    fprintf(stdout, "ACK %s:%d %d\n", inet_ntoa(client.sin_addr), (int) ntohs(client.sin_port), packet_number);
}

void get_parametrs(struct parametrs* p, int argc, char **argv) {
    int c;

    if (argc != 4) {
        fprintf(stderr, "Invalid number of arguments.\n");
        exit(-1);
    }

    while ((c = getopt (argc, argv, "p:")) != -1) {
        switch (c) {
            case 'p':
                p->port = atoi(optarg);
                break;
        }
    }

    p->root_dirpath = argv[argc-1];
}


int main(int argc, char **argv) {

    struct parametrs p;

    // get and control input parametrs
    get_parametrs(&p, argc, argv);

    main_loop(p);

    return 0;
}