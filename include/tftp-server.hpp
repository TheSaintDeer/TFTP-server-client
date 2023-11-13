// Name: Ivan Golikov
// Login: xgolik00

#ifndef SERVER
#define SERVER

#include <bits/stdc++.h> 

#include "packet.hpp"
#include "common.hpp"

using namespace std;

/**
 * The socket to be used to send the packet
*/
int sock;

/**
 * Client socket file descriptor
*/
struct sockaddr_in client;

/**
 * Structure for input parametrs
*/
struct parametrs {
    int port = STANDART_PORT;
    char* root_dirpath;
};

/**
 * The function where the socket is created and the main loop is located
 * @param p input parametrs
*/
void main_loop(struct parametrs p);

/**
 * The function where the descendant from the main loop enters and begins communication with the client
 * @param op operation (RRQ/WRQ)
 * @param addr_len size of address
 * @param recv_len length of received data
 * @param p input parametrs
*/
void processing_request(int op, size_t addr_len, char* buffer, int recv_len, struct parametrs p);

/**
 * Start sending a file
 * @param buffer received data
 * @param o OPTS parametrs
 * @param file the file where the data will be read from
 * @param without_opts communication with OPTS or without
 * @param mode communication mode (octet/netascii)
 * @param c_sock the socket to be used to send the packet
*/
void RRQ_handling(size_t addr_len, struct opts o, FILE *file, int without_opts, char *mode, int c_sock);

/**
 * Loop to send a file in octet mode
 * @param file the file where the data will be read from
 * @param addr_len size of address
 * @param blksize max length of data in a packet
 * @param c_sock the socket to be used to send the packet
*/
void RRQ_octet_loop(FILE *file, size_t addr_len, int blksize, int c_sock);

/**
 * Loop to send a file in netascii mode
 * @param file the file where the data will be read from
 * @param addr_len size of address
 * @param blksize max length of data in a packet
 * @param c_sock the socket to be used to send the packet
*/
void RRQ_netascii_loop(FILE *file, size_t addr_len, int blksize, int c_sock);

/**
 * Start reading a file
 * @param addr_len size of address
 * @param o OPTS parametrs
 * @param file the file where the data will be read from
 * @param c_sock the socket to be used to send the packet
*/
void WRQ_handling(size_t addr_len, struct opts o, FILE* file, int c_sock);


/**
 * Function to open a file in the desired fashion
 * @param op operation (RRQ/WRQ)
 * @param filepath the path to the file
 * @param mode communication mode (octet/netascii)
*/
FILE *open_file(int op, char *filepath, char *mode);

/**
 * A function to determine which of the options are enabled
 * @param op operation (RRQ/WRQ)
 * @param child_sock the socket to be used to send the packet
 * @param file required file
 * @param o OPTS parametrs
*/
void handling_opts(int op, int child_sock, FILE *file, struct opts *o);

/**
 * Function for checking OPTS parametrs
 * @param o OPTS parametrs
 * @param buffer received data
 * @param buffer_len length of already checked data
 * @param recv_len length of received data
*/
int control_opts(struct opts* o, char* buffer, int buffer_len, int recv_len);

/**
 * Function for checking ACK packet
 * @param file required file
 * @param packet_len max length of packet
 * @param addr_len size of address
 * @param packet_number number of packet
 * @param c_sock the socket to be used to send the packet
*/
void control_ACK(FILE *file, int packet_len, size_t addr_len, uint16_t packet_number, int c_sock);

/**
 * Accept and process input parameters to start the client
 * @param p input parametrs
*/
void get_parametrs(struct parametrs* p, int argc, char **argv);

#endif