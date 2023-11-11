#include "../include/tftp-client.hpp"

/**
 * tftp-client -h hostname [-p port] [-f filepath] -t filepath
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
	  	        
	            exit(-1);
        }
    }
}

int control_OACK(struct opts* o, char* buffer, int buffer_len, int recv_len) {
    char option[512];
    char value[512];

    while (buffer_len < recv_len) {
        strcpy(option, buffer+buffer_len);
        buffer_len += strlen(option) + 1;
        strcpy(value, buffer+buffer_len);
        buffer_len += strlen(value) + 1;
        
        if (strcmp(option, "blksize") == 0) {
            if (compare_value_ascii(value, o->blksize))
                return 1;

        } else if (strcmp(option, "timeout") == 0) {
            if (compare_value_ascii(value, o->timeout))
                return 1;

        } else if (strcmp(option, "tsize") == 0) {
            get_and_convert_ascii(value, o->tsize);

        } else {
            return 1;
        }
    }
    return 0;
}

void RRQ_loop(FILE *file, int blksize, int addr_len) {
    uint16_t packet_number;
    char buffer[blksize+4];
    int opcode;
    int recv_len = blksize+4;
    while (recv_len == blksize+4) {
        recv_len = recvfrom(sock, (char *)buffer, blksize+4, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) &addr_len);
        
        memcpy(&opcode, (uint16_t *) &buffer, 2);
        opcode = ntohs(opcode);

        if (opcode == DATA) {
            memcpy(&packet_number, (uint16_t *) &buffer[2], 2);
            packet_number = ntohs(packet_number);

            for (int i = 4; i < recv_len; i++)
                fputc(buffer[i], file);

            send_ACK(sock, packet_number, server);
            memset(buffer, 0, blksize+4);
        } else if (opcode == ERROR) {
            memcpy(&opcode, (uint16_t *) &buffer[2], 2);
            exit(ntohs(opcode));
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
    do {
        dim = fread(&data[i], 1, 1, file);
        i += dim;

        if (i == blksize || dim == 0) {
            fprintf(stdout, "%ld\n", strlen(data));
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

            i = 0;
            packet_number++;
        }
    } while(dim == 1);   
}

void request(int op, struct parametrs p) {
    FILE *file;
    if (op == RRQ) 
        file = fopen(p.dest_filepath, "wb");
    else
        file = fopen(p.filepath, "rb");

    if (file == NULL) {
        fprintf(stdout, "File doesn't exist");
    }

    struct opts o = {"1024", "10", "0"};
    if (op == WRQ) {
        fseek(file, 0, SEEK_END);
        int file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        sprintf(o.tsize, "%d", file_size);
    }

    if (op == RRQ) 
        send_first_request(sock, p.filepath, server, o, op);
    else 
        send_first_request(sock, p.dest_filepath, server, o, op);
    
    int opcode;
    int blksize = stoi(o.blksize);
    char buffer[blksize+4];
    int addr_len = sizeof(server);
    int recv_len = recvfrom(sock, (char *)buffer, blksize+4, MSG_WAITALL, (struct sockaddr *)&server, (socklen_t *) & addr_len);

    memcpy(&opcode, (uint16_t *) &buffer, 2);
    opcode = ntohs(opcode);

    if (opcode == OACK) {
        if (control_OACK(&o, buffer, 2, recv_len)) {
            exit(8);
        }
    } else if (opcode == ERROR) {
        memcpy(&opcode, (uint16_t *) &buffer[2], 2);
        exit(ntohs(opcode));
    }

    if (op == RRQ) {
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        if (pages*page_size < stoi(o.tsize)) {
            exit(3);
        }
        send_ACK(sock, 0, server);
    }

    if (op == WRQ) 
        WRQ_loop(file, blksize, addr_len);
    else     
        RRQ_loop(file, blksize, addr_len);

    fclose(file);   
    shutdown(sock, SHUT_RDWR);
    close(sock);
}

int main(int argc, char **argv) {

    struct parametrs p;
    int operation = WRQ;

    // get and control input parametrs
    get_parametrs(&p, &operation, argc, argv);

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        exit(-2); 

    server.sin_family = AF_INET;
    inet_pton(AF_INET, p.hostname, &server.sin_addr);
    server.sin_port = htons(p.port);

    printf("IP address is: %s\n", inet_ntoa(server.sin_addr));
    printf("port is: %d\n", (int) ntohs(server.sin_port));

    // RRQ/WRQ
    if (operation == WRQ) {
        char path[512];
        scanf("%511s", path);
        p.filepath = path;

        request(WRQ, p);
    } else if (operation == RRQ) {
        request(RRQ, p);
    }

    return 0; 
}