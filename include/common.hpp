#ifndef COMMON
#define COMMON

#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>

void convert_to_ASCII(char c, char* ret);

char convert_from_ASCII(char* c);

int compare_value_ascii(char *src_v, char *desc_v);

void get_and_convert_ascii(char *src_v, char *desc_v);

#endif