/*
 * vnmrclient.c
 * ------------
 * Client program on console host, called from user_go with stimulus setup
 * parameters as input, and sending them to TCP server mribg
 *
 * 
 * Example:
 *
 *  an instance in a vnmr macro calling vnmrclient may look like:
 *      shell(vnmrclient start blockstim, design 3)
 *
 *  From the argument a comma separated message created for mribg
 *      'vnmrclient,start,blockstim,design,3'
 *
 *  mribg parses the message and proceeds with giving out the appropriate
 *  commands. In this instance, launches blockstim with arguments 'design 3'
 *  and since the first 2 element signals sequence start, launches ttlctrl.
 */


#include "common.h"
#include "socketcomm.h"

#define VERBOSE 1

int main(int argc, char **argv) {

    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFS];
    char msg[BUFS];
    int ret;

    // change argv[0] to simply 'vnmrclient'
    strcpy(argv[0],"vnmrclient");
    // input check
    ret = make_msg(msg, argc, argv);
    printf("msg : %s\n",msg);

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

    /* Now read server response */
    n = read(sockfd, buffer, BUFS-1);

    if (n < 0) {
       perror("ERROR reading from socket");
       exit(1);
    }
    if(strncmp(buffer, MSG_ACCEPT, strlen(MSG_ACCEPT)) == 0){
        if(VERBOSE > 0){
            printf("%s\n",buffer);
        }
        close(sockfd);
        return 0;
    } else if(strncmp(buffer, MSG_REJECT, strlen(MSG_REJECT)) == 0){
        if(VERBOSE > 0){
            printf("%s\n",buffer);
        }

        fprintf(stderr, "vnmrclient: message was rejected by server\n");
        close(sockfd);
        return 1;

    } else{
        close(sockfd);
        return -1;
    }

}

