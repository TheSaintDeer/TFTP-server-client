// Name: Ivan Golikov
// Login: xgolik00

#ifndef PACKET
#define PACKET

#include <stdio.h>
#include <arpa/inet.h>

#include "common.hpp"

#define PACKET_SIZE     516
#define MODE            "octet"
#define RRQ             1
#define WRQ             2
#define DATA            3
#define ACK             4
#define ERROR           5
#define OACK            6
#define STANDART_PORT   69

/**
 * Structure for storing OPTS data
*/
struct opts {
    char blksize[10] = "-1";
    char timeout[10] = "-1";
    char tsize[10] = "-1";
};

/**
 * Function for sending the first request (RRQ or WRQ)
 * @param sock the socket to be used to send the packet
 * @param filepath the path to write or read the file on the server
 * @param sockad client socket file descriptor
 * @param o all OPTS parameters
 * @param operation RRQ/WRQ
*/
void send_first_request(int sock, char* filepath, struct sockaddr_in sockad, struct opts o, int operation);

/**
 * Function for sending DATA request
 * @param sock the socket to be used to send the packet
 * @param packet_number variable indicating the package number
 * @param data data to be sent
 * @param len maximum possible buffer length
 * @param sockad client socket file descriptor
*/
void send_DATA(int sock, uint16_t packet_number, char* data, int len, struct sockaddr_in sockad);

/**
 * Function for sending ACK request
 * @param sock the socket to be used to send the packet
 * @param packet_number variable indicating the package number
 * @param sockad client socket file descriptor
*/
void send_ACK(int sock, uint16_t packet_number, struct sockaddr_in sockad);

/**
 * Function for sending ERR request
 * @param sock the socket to be used to send the packet
 * @param error_code variable indicating the error number
 * @param sockad client socket file descriptor
*/
void send_ERR(int sock, uint16_t error_code, struct sockaddr_in sockad);

/**
 * Function for sending OACK request
 * @param sock the socket to be used to send the packet
 * @param o all OPTS parameters
 * @param sockad client socket file descriptor
*/
void send_OACK(int sock, struct opts o, struct sockaddr_in sockad);

void print_log(int sock, sockaddr_in sockad);

#endif