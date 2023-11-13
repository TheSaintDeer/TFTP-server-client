/**
 * Name: Ivan Golikov
 * Login: xgolik00
 * 
 * tftp-client -h hostname [-p port] [-f filepath] -t filepath
 * 
 * -h IP adresa/doménový název vzdáleného serveru
 * -p port vzdáleného serveru
 *      pokud není specifikován předpokládá se výchozí dle specifikace
 * -f cesta ke stahovanému souboru na serveru (download)
 *      pokud není specifikován používá se obsah stdin (upload)
 * -t cesta, pod kterou bude soubor na vzdáleném serveru/lokálně uložen
*/
#include "../include/tftp-client.hpp"

void request(int op, struct parametrs p) {
    // open file with need mode 
    FILE *file;
    if (op == RRQ) {
        create_path(p.dest_filepath);
        file = fopen(p.dest_filepath, "wb");
    } else {
        file = fopen(p.filepath, "rb");
    }

    if (file == NULL) {
        fprintf(stderr, "File doesn't exist\n");
        exit(-4);
    }

    // create OPTS parametrs
    struct opts o = {"1024", "10", "0"};
    if (op == WRQ) {
        fseek(file, 0, SEEK_END);
        int file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        sprintf(o.tsize, "%d", file_size);
    }

    // start communication 
    if (op == RRQ) 
        send_first_request(sock, p.filepath, server, o, op);
    else 
        send_first_request(sock, p.dest_filepath, server, o, op);
    
    int opcode;
    int blksize = stoi(o.blksize);
    char buffer[blksize+4];
    int addr_len = sizeof(server);
    int recv_len = recvfrom(sock, (char *)buffer, blksize+4, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);

    // check an answer from the server
    memcpy(&opcode, (uint16_t *) &buffer, 2);
    opcode = ntohs(opcode);

    if (opcode == OACK) {
        fprintf(stdout, "OACK %s:%d", inet_ntoa(server.sin_addr), (int) ntohs(server.sin_port));
        if (control_OACK(&o, buffer, 2, recv_len))
            exit(8);
    } else if (opcode == ERROR) {
        struct sockaddr_in sa;
        int sa_len = sizeof(sa);
        if (getsockname(sock,(struct sockaddr *)&sa,(socklen_t *)&sa_len))
            exit(-2);
            
        memcpy(&opcode, (uint16_t *) &buffer[2], 2);
        opcode = ntohs(opcode);
        char msg[100];
        strcpy(msg, buffer+4);
        fprintf(stdout, "ERROR %s:%d:%d %d %s", inet_ntoa(server.sin_addr), (int) ntohs(server.sin_port), (int) ntohs(sa.sin_port), opcode, msg);
        exit(opcode);
    }

    // check free memory for writting data
    if (op == RRQ) {
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        if (pages*page_size < stoi(o.tsize))
            exit(3);
        send_ACK(sock, 0, server);
    }

    // start reading/sending file's data
    if (op == WRQ) 
        WRQ_loop(file, blksize, addr_len);
    else     
        RRQ_loop(file, blksize, addr_len);

    fclose(file);   
    shutdown(sock, SHUT_RDWR);
    close(sock);
}

void RRQ_loop(FILE *file, int blksize, int addr_len) {
    uint16_t packet_number;
    char buffer[blksize+4];
    int opcode;
    int recv_len = blksize+4;
    struct sockaddr_in sa;
    int sa_len = sizeof(sa);

    // while the length of the received data is equal to the block size
    while (recv_len == blksize+4) {
        recv_len = recvfrom(sock, (char *)buffer, blksize+4, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) &addr_len);
        
        memcpy(&opcode, (uint16_t *) &buffer, 2);
        opcode = ntohs(opcode);

        if (opcode == DATA) {
            memcpy(&packet_number, (uint16_t *) &buffer[2], 2);
            packet_number = ntohs(packet_number);

            // write data to the file
            for (int i = 4; i < recv_len; i++)
                fputc(buffer[i], file);

            if (getsockname(sock,(struct sockaddr *)&sa,(socklen_t *)&sa_len))
                exit(-2);

            fprintf(stdout, "DATA %s:%d:%d %d\n", inet_ntoa(server.sin_addr), (int) ntohs(server.sin_port), (int) ntohs(sa.sin_port), packet_number);
            send_ACK(sock, packet_number, server);
            memset(buffer, 0, blksize+4);
        } else if (opcode == ERROR) {
            if (getsockname(sock,(struct sockaddr *)&sa,(socklen_t *)&sa_len))
                exit(-2);
                
            memcpy(&opcode, (uint16_t *) &buffer[2], 2);
            opcode = ntohs(opcode);
            char msg[100];
            strcpy(msg, buffer+4);
            fprintf(stdout, "ERROR %s:%d:%d %d %s", inet_ntoa(server.sin_addr), (int) ntohs(server.sin_port), (int) ntohs(sa.sin_port), opcode, msg);
            exit(opcode);
        }
    }
}

void WRQ_loop(FILE *file, int blksize, int addr_len) {
    char data[blksize+1];
    char buffer[blksize+4];
    int opcode;
    int recv_len;
    uint16_t packet_number = 1;
    uint16_t recv_packet_number;
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
            send_DATA(sock, packet_number, data, blksize+4, server);
            memset(data, '\0', blksize+1);
            memset(buffer, '\0', blksize+4);
            recv_len = recvfrom(sock, (char*) buffer, blksize+4, MSG_WAITALL, (struct sockaddr*) &server, (socklen_t*) &addr_len);

            memcpy(&opcode, (uint16_t*) &buffer, 2);
            opcode = ntohs(opcode);

            memcpy(&recv_packet_number, (uint16_t*) &buffer[2], 2);
            recv_packet_number = ntohs(recv_packet_number);
            
            if ((opcode != ACK) || (packet_number != recv_packet_number)) {
                break;
            }
            fprintf(stdout, "ACK %s:%d %d\n", inet_ntoa(server.sin_addr), (int) ntohs(server.sin_port), packet_number);

            i = 0;
            packet_number++;
        }
    } while(dim == 1);   
}

int control_OACK(struct opts* o, char* buffer, int buffer_len, int recv_len) {
    char option[512];
    char value[512];

    while (buffer_len < recv_len) {
        // read option
        strcpy(option, buffer+buffer_len);
        buffer_len += strlen(option) + 1;
        // read option's value
        strcpy(value, buffer+buffer_len);
        buffer_len += strlen(value) + 1;
        
        // compare option and its value
        if (strcmp(option, "blksize") == 0) {
            if (compare_value_ascii(value, o->blksize))
                return 1;
            fprintf(stdout, " blksize=%s", o->blksize);

        } else if (strcmp(option, "timeout") == 0) {
            if (compare_value_ascii(value, o->timeout))
                return 1;
            fprintf(stdout, " timeout=%s", o->timeout);

        } else if (strcmp(option, "tsize") == 0) {
            get_and_convert_ascii(value, o->tsize);
            fprintf(stdout, " timeout=%s", o->timeout);
        } else {
            return 1;
        }
    }
    fprintf(stdout, "\n");
    return 0;
}

void get_parametrs(struct parametrs* p, int* oper, int argc, char **argv) {
    int c;

    while ((c = getopt (argc, argv, ":h:p:f:t:m:")) != -1) {
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
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else
                fprintf(stderr, "Unknown option.\n");
            
            exit(-1);
        }
    }
}

int main(int argc, char **argv) {

    struct parametrs p;
    int operation = WRQ;

    // get and control input parametrs
    get_parametrs(&p, &operation, argc, argv);

    // create socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        exit(-2); 

    server.sin_family = AF_INET;
    inet_pton(AF_INET, p.hostname, &server.sin_addr);
    server.sin_port = htons(p.port);

    // RRQ/WRQ
    if (operation == WRQ) {
        char path[512];
        if (scanf("%511s", path) == 0) {
            exit(-3);
        }
        p.filepath = path;

        request(WRQ, p);
    } else if (operation == RRQ) {
        request(RRQ, p);
    }

    return 0; 
}