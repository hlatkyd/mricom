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


    char msgbuf[BUFS] = {0};
    char *token;
    const char delim[] = ",";
    int i;
    int ret;
    strcpy(msgbuf, msg);
    token = strtok(msgbuf, delim);
    i = 0;
    while(token != NULL){
        strcpy(argv[i], token);
        token = strtok(NULL, delim);
        i++;
    }
    return i;
}

/*
 * Function: send_mribg
 * --------------------
 *  Send string message to mribg via socket
 */

#define SEND_VERBOSE 1
int send_mribg(char *msg){

    // note: almost same code as vnmrclient
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFS];
    int ret;

    portno = MRIBGPORT; // 8080

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
       perror("ERROR opening socket");
       exit(1);
    }

    server = gethostbyname(MRIBGHOST);
    if (server == NULL) {
       fprintf(stderr,"ERROR, no such host\n");
       exit(0);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
            (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    ret = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(ret < 0){
       perror("ERROR connecting");
       exit(1);
    }

    /* Send message to the server */

    memset(buffer, 0, BUFS);
    snprintf(buffer, sizeof(buffer), "%s",msg);
    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0) {
       perror("ERROR writing to socket");
       exit(1);
    }
    if(strncmp(buffer, MSG_ACCEPT, strlen(MSG_ACCEPT)) == 0){
        if(SEND_VERBOSE > 0){
            printf("%s\n",buffer);
        }
        return 0;

    } else if(strncmp(buffer, MSG_REJECT, strlen(MSG_REJECT)) == 0){
        if(SEND_VERBOSE > 0){
            printf("%s\n",buffer);
        }

        fprintf(stderr, "send_mribg: message was not processed by server\n");
        return 1;

    } else{

        return -1;
    }
}

