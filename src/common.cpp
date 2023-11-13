// Name: Ivan Golikov
// Login: xgolik00

#include "../include/common.hpp"

/* A function designed to convert character into an ascii code
*/
void convert_to_ASCII(char c, char* ret) {
    switch(c) {
        case '0':
            strcpy(ret, "48");
            break;
        case '1':
            strcpy(ret, "49");
            break;
        case '2':
            strcpy(ret, "50");
            break;
        case '3':
            strcpy(ret, "51");
            break;
        case '4':
            strcpy(ret, "52");
            break;
        case '5':
            strcpy(ret, "53");
            break;
        case '6':
            strcpy(ret, "54");
            break;
        case '7':
            strcpy(ret, "55");
            break;
        case '8':
            strcpy(ret, "56");
            break;
        case '9':
            strcpy(ret, "57");
            break;
    }
}

/* A function designed to convert an ascii code into character
*/
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
    exit(-1);
}

/* Copying every two characters from the string src_v to the variable number 
and then comparing with the character from the variable desc_v
*/
int compare_value_ascii(char *src_v, char *dest_v) {
    char number[3];
    for (int i = 0; i < strlen(src_v) - 1; i += 2){
        memset(number, '\0', sizeof(number));
        strncpy(number, src_v+i, 2);
        if (dest_v[i/2] != convert_from_ASCII(number)) {
            return 1;
        }
    }
    return 0;
}

/*Copying every two characters from the string src_v to the variable number 
and then copying the results to the variable desc_v
*/
void get_and_convert_ascii(char *src_v, char *dest_v) {
    char result[10];
    memset(result, '\0', sizeof(result));
    char number[3];
    for (int i = 0; i < strlen(src_v) - 1; i += 2){
        memset(number, '\0', sizeof(number));
        strncpy(number, src_v+i, 2);
        result[i/2] = convert_from_ASCII(number);
    }
    strcpy(dest_v, result);
}

void create_path(char *filepath) {
    char copy_filepath[512];
    strcpy(copy_filepath, filepath);
    char *token = strtok(copy_filepath, "/");
    char *tmp_token = token;
    char path[512];

    memset(path, '\0', 512);
    token = strtok(NULL, "/");

    while (token != NULL) {
        strcat(path, tmp_token);
        strcat(path, "/");
        mkdir(path, 0777);
        tmp_token = token;
        token = strtok(NULL, "/");
    }
}