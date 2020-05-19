/*
 * mribg.c
 *
 * Background control program for mricom automation
 *
 */

#include "mribg.h"
#include "socketcomm.h"

#define VERBOSE 1

int main(int argc, char **argv){

    // pid setup
    struct mpid *mp;
    struct gen_settings *gs;

    // fifo setup
    char mricomdir[LPATH];
    char bginfifo[LPATH] = {0};
    char bgoutfifo[LPATH] = {0};
    char init_msg[] = "mribg init successful...";
    fd_set set; 
    struct timeval timeout;
    int fd, ret;
    int rv;

    //socket declare
    int sockfd, newsockfd, portno, clilen;
    char buffer[BUFS];
    struct sockaddr_in serv_addr, cli_addr;
    int nread;
    memset(buffer, 0, sizeof(buffer));

    char msg_accept[] = MSG_ACCEPT;
    char msg_reject[] = MSG_REJECT;


    if(getenv("MRICOMDIR") == NULL){
        fprintf(stderr, "mribg: environment varieble MRICOMDIR not set\n");
        exit(1);
    }

    signal(SIGTERM, sighandler);
    strcpy(mricomdir, getenv("MRICOMDIR"));

    // init
    gs = malloc(sizeof(struct gen_settings));
    mp = malloc(sizeof(struct mpid));
    memset(gs, 0, sizeof(struct gen_settings));
    memset(mp, 0, sizeof(struct mpid));
    parse_gen_settings(gs);
    fill_mpid(mp);
    processctrl_add(gs->mpid_file, mp, "START");

    // init fifo
    // TODO make obsolete, do this with unix socket

    snprintf(bginfifo, sizeof(bginfifo), "%s/%s",mricomdir, BGINFIFO);
    snprintf(bgoutfifo, sizeof(bgoutfifo), "%s/%s",mricomdir, BGOUTFIFO);
    mkfifo(bginfifo, 0666);
    mkfifo(bgoutfifo, 0666);

    // socket setup
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("mribg: error opening socket");
        exit(1);
    }
    // enable reuse socket
    int reuse = 1;
    ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
    if(ret < 0){
        perror("setsockopt(SO_REUSEADDR) failed");
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    portno = MRIBGPORT; // see common.h 8080

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    // bind socket
    ret = bind(sockfd, (struct sockaddr * ) &serv_addr, sizeof(serv_addr));
    if(ret < 0){
        perror("mribg: error binding socket");
        exit(1);
    }
    //signal back to mricom
    fd = open(bgoutfifo, O_WRONLY);
    write(fd, init_msg, strlen(init_msg));
    close(fd);

    // start listening
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // accept connection request
    while(1){
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0){
            perror("mribg: error on accept");
            exit(1);
        }
        nread = read(newsockfd, buffer, BUFS-1);
        if(VERBOSE > 0){
            fprintf(stderr, "\n[mribg]: incoming request: '%s'",buffer);
        }

        // do processing
        //ret = process_request(buffer);
        ret = 0;

        // signal back
        if(ret < 0){
            write(newsockfd, msg_reject, sizeof(msg_reject));
            if(VERBOSE > 0){
                fprintf(stderr, "\n[mribg]: request denied");
            }
        } else if(ret == 0){
            write(newsockfd, msg_accept, sizeof(msg_accept));
            if(VERBOSE > 0){
                fprintf(stderr, "\n[mribg]: request accepted");
            }
        }
        close(newsockfd);

        memset(buffer, 0, BUFS);
    }


    // shutdown
    processctrl_add(gs->mpid_file, mp, "STOP");
    return 0;

}


/*
 * Function: process_request
 * -------------------------
 *  General client request handling function
 */

#define MAXARG 16
#define MAXLEN 16
int process_request(char *msg){

    int argc;
    char **argv;
    argv = malloc(sizeof(char) * MAXARG * MAXLEN);
    memset(argv, 0, sizeof(char) * MAXARG * MAXLEN);

    argc = parse_msg(msg, argv);
    return 0;
}
