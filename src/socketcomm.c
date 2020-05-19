/*
 * socketcomm.c
 *
 */
#include "common.h"
#include "socketcomm.h"

/*
 * Function: make_msg
 * ------------------
 *  Concatenate string array into one comma delimited string, return length
 */
int make_msg(char *msg, int argc, char **argv){

    int i;
    int len = 0;
    char buff[BUFS] = {0};
    for(i=0; i < argc; i++){
        strcat(buff, argv[i]);
        if(i<argc-1)
            strcat(buff, ",");
        len += strlen(argv[i]);
    }
    if(len > BUFS){
        fprintf(stderr, "make_msg: buffer overflow\n");
        return -1;
    }
    memset(msg, 0, sizeof(char) * BUFS);
    strcpy(msg, buff);
    return len;
}

/*
 * Function: parse_msg
 * ------------------
 *  Sort delimited string into string array, return number of strings
 */
int parse_msg(char *msg, char **argv){


    char buff[BUFS] = {0};
    char *token;
    const char delim[] = ",";
    int i;
    token = strtok(msg, delim);
    i = 0;
    while(token != NULL){
        strcpy(argv[i], token);
        token = strtok(NULL, delim);
        i++;
    }
    return i;
}
