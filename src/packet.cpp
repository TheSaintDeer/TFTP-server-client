#include "../include/packet.hpp"

void send_RRQ(int sock, char* filepath, struct sockaddr_in sockad) {
    char buffer[1024];
    int buffer_len = 0;
    uint16_t opcode = htons(RRQ);
    uint8_t end_string = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    strcpy(buffer+buffer_len, filepath);
    buffer_len += strlen(filepath);

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    strcpy(buffer+buffer_len, MODE);
    buffer_len += strlen(MODE);

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0) {
        exit(2);
    }
}

void send_WRQ(int sock, char* filepath, struct sockaddr_in sockad) {
    int opcode = htons(WRQ);
    char buffer[PACKET_SIZE];
    int buffer_len = 0;
    int end_string = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    strcpy(buffer+buffer_len, filepath);
    buffer_len += strlen(filepath);

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    strcpy(buffer+buffer_len, MODE);
    buffer_len += strlen(MODE);

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&sockad, sizeof(sockad)) < 0) {
        exit(2);
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
    char error_msg[DATA_SIZE];

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    memcpy(buffer+buffer_len, &error_code, 2);
    buffer_len += 2;

    switch (error_code) {
        case NOT_DEFINED:
            strcpy(error_msg, "Not defined.\n");
            break;
        case FILE_NOT_FOUND:
            strcpy(error_msg, "File not found.\n");
            break;
        case ACCESS_VIOLATION:
            strcpy(error_msg, "Access violation.\n");
            break;
        case DISK_FULL:
            strcpy(error_msg, "Disk full or allocation exceeded.\n");
            break;
        case ILLEGAL_TFTP_OPERATION:
            strcpy(error_msg, "Illegal TFTP operation.\n");
            break;
        case UNKNOWN_TRANSFER_ID:
            strcpy(error_msg, "Unknown transfer ID.\n");
            break;
        case FILE_ALREADY_EXIST:
            strcpy(error_msg, "File already exists.\n");
            break;
        case NO_SUCH_USER:
            strcpy(error_msg, "No such user.\n");
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