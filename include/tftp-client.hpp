// Name: Ivan Golikov
// Login: xgolik00

#ifndef CLIENT
#define CLIENT

#include <iostream>
#include <unistd.h>

#include "packet.hpp"
#include "common.hpp"

using namespace std;

/**
 * The socket to be used to send the packet
*/
int sock;

/**
 * Server socket file descriptor
*/
sockaddr_in server;

/**
 * Structure for input parametrs
*/
struct parametrs {
    char* hostname;
    int port = STANDART_PORT;
    char* filepath;
    char* dest_filepath;
};

/**
 * The function of starting communication between the client and the server
 * @param op operation (RRQ/WRQ)
 * @param p input parametrs
*/
void request(int op, struct parametrs p);

/**
 * A function with a main loop for accepting a file from the server
 * @param file the file where the data will be written
 * @param blksize max length of data in a packet
 * @param addr_len size of address
*/
void RRQ_loop(FILE *file, int blksize, int addr_len);

/**
 * A function with a main loop for accepting a file from the server
 * @param file the file where the data will be read from
 * @param blksize max length of data in a packet
 * @param addr_len size of address
*/
void WRQ_loop(FILE *file, int blksize, int addr_len);

/**
 * Function for checking the OACK packet
 * @param o OPTS parametrs
 * @param buffer received data
 * @param buffer_len length of already checked data
 * @param recv_len length of received data
*/
int control_OACK(struct opts* o, char* buffer, int buffer_len, int recv_len);

/**
 * Accept and process input parameters to start the client
 * @param p input parametrs
 * @param oper operation RRW or WRQ
*/
void get_parametrs(struct parametrs* p, int* oper, int argc, char **argv);

#endif