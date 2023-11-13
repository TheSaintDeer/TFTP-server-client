// Name: Ivan Golikov
// Login: xgolik00

#include "../include/packet.hpp"

/* A function for sending the first request to start communication between the client and the server
*/
void send_first_request(int sock, char* filepath, struct sockaddr_in sockad, struct opts o, int operation) {
    char buffer[1024];
    char mode[10] = MODE;
    int buffer_len = 0;
    uint16_t opcode = htons(operation);
    uint8_t end_string = 0;
    int i;
    int n;
    char c[2];

    // opcode
    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    // filename
    strcpy(buffer+buffer_len, filepath);
    buffer_len += strlen(filepath);

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    // mode
    strcpy(buffer+buffer_len, mode);
    buffer_len += strlen(mode);

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    // block size
    strcpy(buffer+buffer_len, "blksize");
    buffer_len += strlen("blksize");

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    for (int i = 0; i < strlen(o.blksize); i++) {
        
        convert_to_ASCII(o.blksize[i], c);
        strcpy(buffer+buffer_len, c);
        buffer_len += strlen(c);
    }

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    // timeout
    strcpy(buffer+buffer_len, "timeout");
    buffer_len += strlen("timeout");

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    for (int i = 0; i < strlen(o.timeout); i++) {
        convert_to_ASCII(o.timeout[i], c);
        strcpy(buffer+buffer_len, c);
        buffer_len += strlen(c);
    }

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    // file's size
    strcpy(buffer+buffer_len, "tsize");
    buffer_len += strlen("tsize");

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    for (int i = 0; i < strlen(o.tsize); i++) {
        convert_to_ASCII(o.tsize[i], c);
        strcpy(buffer+buffer_len, c);
        buffer_len += strlen(c);
    }

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    if (buffer_len > 512) {
        fprintf(stderr, "Too long request packet.\n");
        exit(-1);
    }

    // sending
    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0)
        exit(-1);
}

/* A function for sending DATA request
*/
void send_DATA(int sock, uint16_t packet_number, char* data, int len, struct sockaddr_in sockad) {
    int opcode = htons(DATA);
    char buffer[len];
    int buffer_len = 0;

    // opcode
    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    // packet number
    packet_number = htons(packet_number);
    memcpy(buffer+buffer_len, &packet_number, 2);
    buffer_len += 2;

    strcpy(buffer+buffer_len, data);
    buffer_len += strlen(data);

    // sending
    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0)
        exit(-1);
}

/* A function for sending ACK request
*/
void send_ACK(int sock, uint16_t packet_number, struct sockaddr_in sockad) {
    int opcode = htons(ACK);
    char buffer[PACKET_SIZE];
    int buffer_len = 0;

    // opcode
    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    // packet number
    packet_number = htons(packet_number);
    memcpy(buffer+buffer_len, &packet_number, 2);
    buffer_len += 2;

    // sending
    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0)
        exit(-1);
}

/* A function for sending ERROR request
*/
void send_ERR(int sock, uint16_t error_code, struct sockaddr_in sockad) {
    int opcode = htons(ERROR);
    char buffer[PACKET_SIZE];
    int buffer_len = 0;
    int end_string = 0;
    char error_msg[PACKET_SIZE-4];

    // opcode
    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    // error text definition
    switch (error_code) {
        case 0:
            strcpy(error_msg, "Not defined.\n");
            break;
        case 1:
            strcpy(error_msg, "File not found.\n");
            break;
        case 2:
            strcpy(error_msg, "Access violation.\n");
            break;
        case 3:
            strcpy(error_msg, "Disk full or allocation exceeded.\n");
            break;
        case 4:
            strcpy(error_msg, "Illegal TFTP operation.\n");
            break;
        case 5:
            strcpy(error_msg, "Unknown transfer ID.\n");
            break;
        case 6:
            strcpy(error_msg, "File already exists.\n");
            break;
        case 7:
            strcpy(error_msg, "No such user.\n");
            break;
        case 8:
            strcpy(error_msg, "Transfer should be terminated due to option negotiation.\n");
            break;
    }

    // code error
	error_code = htons(error_code);
    memcpy(buffer+buffer_len, &error_code, 2);
    buffer_len += 2;

    // error message
    strcpy(buffer+buffer_len, error_msg);
    buffer_len += strlen(error_msg);

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    // sending
    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0)
        exit(-1);
}

/* A function for sending OACK request
*/
void send_OACK(int sock, struct opts o, struct sockaddr_in sockad) {
    char buffer[1024];
    int buffer_len = 0;
    uint16_t opcode = htons(OACK);
    uint8_t end_string = 0;
    int i;
    int n;
    char c[2];

    // opcode
    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    // block size
    if (strcmp(o.blksize, "-1") != 0) {
        strcpy(buffer+buffer_len, "blksize");
        buffer_len += strlen("blksize");

        memcpy(buffer+buffer_len, &end_string, 1);
        buffer_len++;

        for (int i = 0; i < strlen(o.blksize); i++) {
            convert_to_ASCII(o.blksize[i], c);
            strcpy(buffer+buffer_len, c);
            buffer_len += strlen(c);
        }

        memcpy(buffer+buffer_len, &end_string, 1);
        buffer_len++;
    }

    // timeout
    if (strcmp(o.timeout, "-1") != 0) {
        strcpy(buffer+buffer_len, "timeout");
        buffer_len += strlen("timeout");

        memcpy(buffer+buffer_len, &end_string, 1);
        buffer_len++;

        for (int i = 0; i < strlen(o.timeout); i++) {
            convert_to_ASCII(o.timeout[i], c);
            strcpy(buffer+buffer_len, c);
            buffer_len += strlen(c);
        }

        memcpy(buffer+buffer_len, &end_string, 1);
        buffer_len++;
    }

    // file's size
    if (strcmp(o.tsize, "-1") != 0) {
        strcpy(buffer+buffer_len, "tsize");
        buffer_len += strlen("tsize");

        memcpy(buffer+buffer_len, &end_string, 1);
        buffer_len++;

        for (int i = 0; i < strlen(o.tsize); i++) {
            convert_to_ASCII(o.tsize[i], c);
            strcpy(buffer+buffer_len, c);
            buffer_len += strlen(c);
        }

        memcpy(buffer+buffer_len, &end_string, 1);
        buffer_len++;
    }

    // sending
    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0)
        exit(-1);
}

// void print_log(int sock, sockaddr_in sockad, char *buffer) {
//     struct sockaddr_in sa;
//     int sa_len = sizeof(sa);

//     if (getsockname(sock,(struct sockaddr *)&sa,(socklen_t *)&sa_len))
//         exit(-2);

//     char mode[10];
//     int buffer_len = 0;
//     uint16_t opcode;    

//     memcpy(buffer, &opcode, 2);
//     buffer_len += 2;

//     // filename
//     strcpy(buffer+buffer_len, filepath);
//     buffer_len += strlen(filepath);

//     memcpy(buffer+buffer_len, &end_string, 1);
//     buffer_len++;

//     // mode
//     strcpy(buffer+buffer_len, mode);
//     buffer_len += strlen(mode);

//     memcpy(buffer+buffer_len, &end_string, 1);
//     buffer_len++;

//     fprintf(stdout, "RRQ %s:%d \"%s\" %s blksize=%s timeout=%s tsize=%s\n", inet_ntoa(sa.sin_addr), (int) ntohs(sa.sin_port), filepath, mode);
// }