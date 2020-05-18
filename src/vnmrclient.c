/*
 * vnmrclient.c
 * ------------
 * Client program on console host, receiving message from vnmrpipe and
 * sending it to TCP server mribg
 */


#include "common.h"
#include "vnmrcommon.h"

#define BUFS 
#define LOCALHOST "127.0.0.1"
int main(int argc, char **argv) {

    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[BUFS];

    portno = BGSPORT;

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
       perror("ERROR opening socket");
       exit(1);
    }
     
    server = gethostbyname(argv[1]);

    if (server == NULL) {
       fprintf(stderr,"ERROR, no such host\n");
       exit(0);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
       perror("ERROR connecting");
       exit(1);
    }

    /* Now ask for a message from the user, this message
       * will be read by server
    */
     
    snprintf(buffer, sizeof(buffer), "%s",msg);

    /* Send message to the server */
    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0) {
       perror("ERROR writing to socket");
       exit(1);
    }

    /* Now read server response */
    memset(buffer, 0, BUFS);
    n = read(sockfd, buffer, BUFS-1);

    if (n < 0) {
       perror("ERROR reading from socket");
       exit(1);
    }
     
    printf("%s\n",buffer);
    return 0;

}
