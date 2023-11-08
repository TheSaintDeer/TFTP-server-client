#include "../include/tftp-client.hpp"

/**
 * tftp-client -h hostname [-p port] [-f filepath] -t dest_filepath
 * 
 * -h IP adresa/doménový název vzdáleného serveru
 * -p port vzdáleného serveru
 *      pokud není specifikován předpokládá se výchozí dle specifikace
 * -f cesta ke stahovanému souboru na serveru (download)
 *      pokud není specifikován používá se obsah stdin (upload)
 * -t cesta, pod kterou bude soubor na vzdáleném serveru/lokálně uložen
 * -m nastavi mod (octet/netascii)
*/
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
	  	        
	            exit(1);
        }
    }
}

int check_OACK(struct opts* o, char* buffer, int buffer_len, int recv_len) {
    char option[512];
    char value[512];

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
                if (o->blksize[i/2] != convert_from_ASCII(number)) {
                    return 1;
                }
            }
        } else if (strcmp(option, "timeout") == 0) {
            strcpy(value, buffer+buffer_len);
            buffer_len += strlen(value) + 1;
            char number[3];
            for (int i = 0; i < strlen(value) - 1; i += 2){
                memset(number, '\0', sizeof(number));
                strncpy(number, value+i, 2);
                if (o->timeout[i/2] != convert_from_ASCII(number)) {
                    return 1;
                }
            }
        } else if (strcmp(option, "tsize") == 0) {
            char result[10];
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
    }
    return 0;
}

void RRQ_request(int sock, sockaddr_in server, struct parametrs p) {
    FILE *dest_file = fopen(p.dest_filepath, "w+b");

    if (dest_file == NULL) {
        fprintf(stdout, "log: could not open file.\n");
        exit(1);
    }

    struct opts o = {"1024", "10", "0"};
    send_first_request(sock, p.filepath, server, o, RRQ);

    int opcode;
    int blksize = stoi(o.blksize);
    char buffer[blksize+4];
    int buffer_len = 0;
    int addr_len = sizeof(server);
    int recv_len = recvfrom(sock, (char *)buffer, blksize+4, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);

    memcpy(&opcode, (uint16_t *) &buffer, 2);
    buffer_len += 2;
    opcode = ntohs(opcode);

    if (opcode == OACK) {
    
    } else if (opcode == ERROR) {
        exit(1);
    } else {
        fprintf(stderr, "Received not OACK packet.\n");
        exit(1);
    }

    // checking computer's memory
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    if (pages*page_size < stoi(o.tsize)) {
        send_ERR(sock, 3, server);
        exit(1);
    }


    if (check_OACK(&o, buffer, buffer_len, recv_len)) {
        send_ERR(sock, 8, server);
        exit(1);
    } else
        send_ACK(sock, 0, server);

    memset(buffer, 0, blksize+4);
    uint16_t packet_number;
    recv_len = blksize+4;
    while (recv_len == blksize+4) {
        recv_len = recvfrom(sock, (char *)buffer, blksize+4, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) &addr_len);
        
        memcpy(&opcode, (uint16_t *) &buffer, 2);
        opcode = ntohs(opcode);

        if (opcode == DATA) {
            memcpy(&packet_number, (uint16_t *) & buffer[2], 2);
            packet_number = ntohs(packet_number);

            for (int i = 4; i < recv_len; i++)
                fputc(buffer[i], dest_file);

            send_ACK(sock, packet_number, server);
            memset(buffer, 0, blksize+4);
        } else if (opcode == ERROR) {
            exit(1);
        }
    }
    shutdown(sock, SHUT_RDWR);
    close(sock);
    fclose(dest_file);   
}

void WRQ_request(int sock, sockaddr_in server, struct parametrs p) {
    FILE *dest_file = fopen(p.dest_filepath, "r+b");

    if (dest_file == NULL) {
        fprintf(stdout, "log: could not open file.\n");
        exit(1);
    }

    fseek(dest_file, 0, SEEK_END);
    int file_size = ftell(dest_file);
    fseek(dest_file, 0, SEEK_SET);

    char chars_filesize[10];
    sprintf(chars_filesize, "%d", file_size);

    struct opts o = {"1024", "10", "0"};
    strcpy(o.tsize, chars_filesize);
    send_first_request(sock, p.filepath, server, o, WRQ);

    int opcode;
    int blksize = stoi(o.blksize);
    char buffer[blksize+4];
    int buffer_len = 0;
    int addr_len = sizeof(server);
    int recv_len = recvfrom(sock, (char *)buffer, blksize+4, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);

    memcpy(&opcode, (uint16_t *) &buffer, 2);
    buffer_len += 2;
    opcode = ntohs(opcode);

    if (opcode == OACK) {
    
    } else if (opcode == ERROR) {
        exit(1);
    } else {
        fprintf(stderr, "Received not OACK packet.\n");
        exit(1);
    }

    if (check_OACK(&o, buffer, buffer_len, recv_len))
        send_ERR(sock, 8, server);

    uint16_t packet_number = 1;
    uint16_t recv_packet_number;

    char data[blksize];
    size_t dim = 0;
    int i = 0;
    do {
        dim = fread(&data[i], 1, 1, dest_file);
        i += dim;

        if (i == blksize || dim == 0) {
            send_DATA(sock, packet_number, data, blksize+4, server);
            memset(data, 0, blksize);
            memset(buffer, 0, blksize+4);
            recv_len = recvfrom(sock, (char*) buffer, blksize+4, MSG_WAITALL, (struct sockaddr*) &server, (socklen_t*) &addr_len);

            memcpy(&opcode, (uint16_t*) &buffer, 2);
            opcode = ntohs(opcode);

            if (opcode != ACK) {
                fclose(dest_file);
                shutdown(sock, SHUT_RDWR);
                close(sock);
                exit(1);
            }

            memcpy(&recv_packet_number, (uint16_t*) &buffer[2], 2);
            recv_packet_number = ntohs(recv_packet_number);
            
            if (packet_number != recv_packet_number) {
                fclose(dest_file);
                shutdown(sock, SHUT_RDWR);
                close(sock);
                exit(1);
            }

            i = 0;
            packet_number++;
        }
    } while(dim == 1);   

    shutdown(sock, SHUT_RDWR);
    close(sock);
    exit(1);     
}

int main(int argc, char **argv) {

    struct parametrs p;
    int operation = WRQ;

    // get and control input parametrs
    get_parametrs(&p, &operation, argc, argv);

    int sock;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        exit(1); 
    } 

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    inet_pton(AF_INET, p.hostname, &server.sin_addr);
    server.sin_port = htons(p.port);

    // RRQ/WRQ
    if (operation == WRQ) {
        char path[512];
        scanf("%511s", path);
        p.filepath = path;

        WRQ_request(sock, server, p);
    } else if (operation == RRQ) {
        RRQ_request(sock, server, p);
    }

    return 0; 
}