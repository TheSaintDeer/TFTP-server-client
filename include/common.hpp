// Name: Ivan Golikov
// Login: xgolik00

#ifndef COMMON
#define COMMON

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

/**
 * A function designed to convert character into an ascii code
 * @param c input character
 * @param ret output ascii code
*/
void convert_to_ASCII(char c, char* ret);

/**
 * A function designed to convert an ascii code into character
 * @param c input ascii code
 * @return output character
*/
char convert_from_ASCII(char* c);

/**
 * A function for comparing a string consisting of characters and a string consisting of an ascii
 * @param src_v string consisting of an ascii
 * @param desc_v string consisting of characters
 * @return 1 - don't match, 0 - match
*/
int compare_value_ascii(char *src_v, char *desc_v);

/**
 * A function for converting a string consisting of an ascii and copying the result to another variable
 * @param src_v string consisting of an ascii
 * @param desc_v the result
*/
void get_and_convert_ascii(char *src_v, char *desc_v);

/**
 * A function for create the path to a file without the file name itself
 * @param filepath Full file name
*/
void create_path(char *filepath);

#endif