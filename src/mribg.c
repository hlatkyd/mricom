/*
 * mribg.c
 *
 * Background control program for mricom automation
 *
 */

#include "mribg.h"
#define BUFS 256
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
    fd = open(bgoutfifo, O_WRONLY);
    write(fd, init_msg, strlen(init_msg));
    close(fd);

    // socket setup
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("mribg: error opening socket");
        exit(1);
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
    portno = BGSPORT;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    // bind socket
    if(bind(sockfd, (struct sockaddr * ) &serv_addr, sizeof(serv_addr))<0){
        perror("mribg: error binding socket");
        exit(1);
    }
    // start listening
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // accept connection request
    while(1){
        memset(buffer, 0, sizeof(buffer));
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if(newsockfd < 0){
            perror("mribg: error on accept");
            exit(1);
        }
        char msg_back[] = "got it, thanks!";
        memset(buffer, 0, sizeof(buffer));
        nread = read(newsockfd, buffer, BUFS-1);
        printf("got message: %s\n",buffer);
        write(newsockfd, msg_back, sizeof(msg_back));
        close(newsockfd);
        sleep(1);
    }



    // shutdown
    processctrl_add(gs->mpid_file, mp, "STOP");
    return 0;

}
