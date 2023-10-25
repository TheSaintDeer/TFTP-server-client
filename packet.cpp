#include "packet.hpp"

void send_RRQ(int sock, char* filepath, struct sockaddr_in server) {
    char buffer[1024];
    int buffer_len = 0;
    uint16_t opcode = htons(READ);
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

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&server, sizeof(server)) < 0) {
        exit(2);
    }
}

void send_WRQ(int sock, char* filepath, struct sockaddr_in server) {
    int opcode = htons(WRITE);
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

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&server, sizeof(server)) < 0) {
        exit(2);
    }

}

void send_DATA(int sock, uint16_t packet_number, char* data, struct sockaddr_in server) {
    int opcode = htons(DATA);
    char buffer[PACKET_SIZE];
    int buffer_len = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    packet_number = htons(packet_number);
    memcpy(buffer+buffer_len, &packet_number, 2);
    buffer_len += 2;

    fprintf(stdout, "Data: %s\n", data);
    strcpy(buffer+buffer_len, data);
    buffer_len += strlen(data);

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&server, sizeof(server)) < 0) {
        exit(2);
    }
}

void send_ACK(int sock, uint16_t packet_number, struct sockaddr_in server) {
    int opcode = htons(ACK);
    char buffer[PACKET_SIZE];
    int buffer_len = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    packet_number = htons(packet_number);
    memcpy(buffer+buffer_len, &packet_number, 2);
    buffer_len += 2;

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&server, sizeof(server)) < 0) {
        exit(2);
    }
}

void send_ERR(int sock, uint16_t error_code, char* error_msg, struct sockaddr_in server) {
    int opcode = htons(ERROR);
    char buffer[PACKET_SIZE];
    int buffer_len = 0;
	error_code = htons(error_code);
    int end_string = 0;

    memcpy(buffer, &opcode, 2);
    buffer_len += 2;

    memcpy(buffer+buffer_len, &error_code, 2);
    buffer_len += 2;

    strcpy(buffer+buffer_len, error_msg);
    buffer_len += strlen(error_msg);

    memcpy(buffer+buffer_len, &end_string, 1);
    buffer_len++;

    if (sendto(sock, buffer, buffer_len, MSG_CONFIRM, (const struct sockaddr *)&server, sizeof(server)) < 0) {
        exit(2);
    }
}