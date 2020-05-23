/*
 * mribg.c
 *
 * Background control program for mricom automation
 *
 */

#include "mribg.h"

#define VERBOSE 1


char status[33] = {0};
//TODO
/* possible values:
 * "ready"
 * "auto_experiment_running"
 * "auto_waiting"
 * 
 */

int main(int argc, char **argv){

    // pid setup
    struct mpid *mp;
    struct gen_settings *gs;

    // fifo setup
    char mricomdir[LPATH];
    int ret;

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
            fprintf(stderr, "[mribg]: incoming request: '%s'\n",buffer);
        }

        // do processing
        ret = process_request(buffer);

        // signal back
        if(ret < 0){
            if(VERBOSE > 0){
                fprintf(stderr, "[mribg]: request denied\n");
            }
            write(newsockfd, msg_reject, sizeof(msg_reject));
        } else if(ret > 0){
            if(VERBOSE > 0){
                fprintf(stderr, "[mribg]: request accepted\n");
            }
            write(newsockfd, msg_accept, sizeof(msg_accept));
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

int process_request(char *msg){

    int argc;
    char **argv;
    char **cmdargv;
    int i;
    pid_t pid;

    argv = calloc(MAXARG,sizeof(char*));
    for(i=0; i< MAXARG; i++){
        argv[i] = calloc(MAXLEN,sizeof(char));
    }
    // this is for passing as command line arguments
    cmdargv = calloc(MAXARG,sizeof(char*));
    for(i=0; i< MAXARG; i++){
        cmdargv[i] = calloc(MAXLEN,sizeof(char));
    }

    argc = parse_msg(msg, argv, ",");
    // check input
    
    if(strncmp(argv[0],"start",5) == 0){
        if(strncmp(argv[1], "blockstim",9)==0){
            for(i=0;i<argc;i++){
                strcpy(cmdargv[i],argv[i+1]);
            }
            fork_blockstim(cmdargv);
            return 1;
        }
            
    }
    return -1;
}

/*
 * Function: frok_blockstim
 * ------------------------
 *  Launch blockstim with arguments
 */
int fork_blockstim(char **args){

    char path[LPATH];
    char mricomdir[LPATH] = {0};
    strcpy(mricomdir,getenv("MRICOMDIR"));
    int ret;
    pid_t p;
    snprintf(path, sizeof(path),"%s/%sblockstim",mricomdir,BIN_DIR);

    //fprintf(stderr, "path %s\n",path);
    p = fork();

    if(p < 0){
        fprintf(stderr, "mribg_launch: fork failed");
        exit(1);
    // parent process
    } else if(p > 0) {

        return p;

    // child process
    } else {

        // launch
        char *cmdargs[4];
        cmdargs[0] = path;
        cmdargs[1] = "design";
        cmdargs[2] = args[2];
        cmdargs[3] = NULL;
        ret = execvp(path,cmdargs);
        if(ret < 0){
            perror("fork_blockstim: execvp");
            exit(1);
        }
        return 0;
    }
}
