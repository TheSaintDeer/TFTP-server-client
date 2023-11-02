#include "../include/packet.hpp"

char* convert_to_ASCII(char c) {
    switch(c) {
        case '0':
            return "48";
        case '1':
            return "49";
        case '2':
            return "50";
        case '3':
            return "51";
        case '4':
            return "52";
        case '5':
            return "53";
        case '6':
            return "54";
        case '7':
            return "55";
        case '8':
            return "56";
        case '9':
            return "57";
    }
}

char convert_from_ASCII(char* c) {
    if (strcmp(c, "48") == 0)
        return '0';
    else if (strcmp(c, "49") == 0)
        return '1';
    else if (strcmp(c, "50") == 0)
        return '2';
    else if (strcmp(c, "51") == 0)
        return '3';
    else if (strcmp(c, "52") == 0)
        return '4';
    else if (strcmp(c, "53") == 0)
        return '5';
    else if (strcmp(c, "54") == 0)
        return '6';
    else if (strcmp(c, "55") == 0)
        return '7';
    else if (strcmp(c, "56") == 0)
        return '8';
    else if (strcmp(c, "57") == 0)
        return '9';
}

void send_first_request(int sock, char* filepath, struct sockaddr_in sockad, struct opts o, int operation) {
    char buffer[1024];
    char mode[10] = MODE;
    int buffer_len = 0;
    uint16_t opcode = htons(operation);
    uint8_t end_string = 0;
    int i;
    int n;

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
        char* c = convert_to_ASCII(o.blksize[i]);
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
        char* c = convert_to_ASCII(o.timeout[i]);
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
        char* c = convert_to_ASCII(o.tsize[i]);
        strcpy(buffer+buffer_len, c);
        buffer_len += strlen(c);
    }

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    if (buffer_len > 512) {
        fprintf(stderr, "Too long request packet.\n");
        exit(-1);
    }

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0) {
        exit(-1);
    }
}

void send_DATA(int sock, uint16_t packet_number, char* data, struct sockaddr_in sockad) {
    int opcode = htons(DATA);
    char buffer[PACKET_SIZE];
    int buffer_len = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    packet_number = htons(packet_number);
    memcpy(buffer+buffer_len, &packet_number, 2);
    buffer_len += 2;

    strcpy(buffer+buffer_len, data);
    buffer_len += strlen(data);

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0) {
        exit(2);
    }
}

void send_ACK(int sock, uint16_t packet_number, struct sockaddr_in sockad) {
    int opcode = htons(ACK);
    char buffer[PACKET_SIZE];
    int buffer_len = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    packet_number = htons(packet_number);
    memcpy(buffer+buffer_len, &packet_number, 2);
    buffer_len += 2;

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0) {
        exit(2);
    }
}

void send_ERR(int sock, uint16_t error_code, struct sockaddr_in sockad) {
    int opcode = htons(ERROR);
    char buffer[PACKET_SIZE];
    int buffer_len = 0;
	error_code = htons(error_code);
    int end_string = 0;
    char error_msg[PACKET_SIZE-4];

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    memcpy(buffer+buffer_len, &error_code, 2);
    buffer_len += 2;

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

    strcpy(buffer+buffer_len, error_msg);
    buffer_len += strlen(error_msg);

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0) {
        exit(2);
    }
}

void send_OACK() {

}