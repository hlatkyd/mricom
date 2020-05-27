/*
 * mribg.c
 *
 * Background control program for mricom automation
 *
 */

#include "mribg.h"

#define VERBOSE 1


int mribg_status = 0;
/* possible value meanings:
 * "0 - manual control from mricom"
 * "1 - automated and waiting"
 * "2 - automated and experiment_running"
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
    char msg_back[BUFS] = {0};


    if(getenv("MRICOMDIR") == NULL){
        fprintf(stderr, "mribg: environment varieble MRICOMDIR not set\n");
        exit(1);
    }

    signal(SIGINT, sighandler);
    signal(SIGUSR1, sighandler);

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

        // do processing, start subprograms, etc
        ret = process_request(buffer, msg_back);

        // signal back if accepted
        if(ret < 0){
            if(VERBOSE > 0){
                fprintf(stderr, "[mribg]: request denied\n");
            }
            write(newsockfd, msg_reject, sizeof(msg_reject));
        // signal back if rejected
        } else if(ret > 0){
            if(VERBOSE > 0){
                fprintf(stderr, "[mribg]: request accepted\n");
            }
            write(newsockfd, msg_accept, sizeof(msg_accept));

        // write direct message if message was a query
        } else if(ret == 0){
            if(VERBOSE > 0){
                fprintf(stderr, "[mribg]: %s\n",msg_back);
            }
            write(newsockfd, msg_back, sizeof(msg_back));
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
 *
 *  Currently handled requests:
 *      start
 *      get
 *      set
 *
 *  Return 1 if request is accepted, -1 if denied
 *  Return 0 if request was a query, and fill response
 *
 *  General format of messages:
 *  [sender name],[request][]
 *  examples:
 *      vnmrclient,start,blockstim,design,test
 *      mricom,get,status
 *      mricom,set,status,1
 *
 */

int process_request(char *msg, char *msg_response){

    int argc;
    char **argv;
    char **cmdargv;
    int i, ret;
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
    
    // -----------------VNMRCLIENT-------------------
    // TODO
    if(strcmp(argv[0],"vnmrclient") == 0){
        // launch ttlctrl
        ;
    }
    // ---------------- START ----------------------
    if(strcmp(argv[1],"start") == 0){
        // check if mribg is in auto mode
        ret = mribg_status_check();
        if(ret < 0){
            return -1;
        }
        if(strcmp(argv[2], "blockstim")==0){
            for(i=0;i<argc;i++){
                // blockstim doesnt need the first 2 argument
                strcpy(cmdargv[i],argv[i+2]);
            }
            fork_blockstim(cmdargv);
            return 1;
        }
        if(strcmp(argv[2], "analogdaq")==0){
            for(i=0;i<argc;i++){
                // blockstim doesnt need the first 2 argument
                strcpy(cmdargv[i],argv[i+2]);
            }
            fork_analogdaq(cmdargv);
            return 1;
        }
            
    }
    
    // ---------------- STOP ----------------------
    if(strcmp(argv[1],"stop") == 0){
        // check if mribg is in auto mode
        ret = mribg_status_check();
        if(ret < 0){
            return -1;
        }
        if(strcmp(argv[2], "blockstim")==0){
            return 1;
        }
            
    }

    // ---------------- GET ----------------------
    if(argc == 3 && strcmp(argv[1],"get") == 0){
        if(strcmp(argv[2], "status")==0){
            memset(msg_response, 0, sizeof(char)*BUFS);
            snprintf(msg_response, BUFS, "%d",mribg_status);
            return 0;
        }
            
    }
    // ---------------- SET ----------------------
    if( argc == 4 && strcmp(argv[1],"set") == 0){
        if(strcmp(argv[2], "status")==0){

            mribg_status = atoi(argv[3]);
            return 1;
        }
            
    }
    return -1;
}

/*
 * Function: fork_blockstim
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

/*
 * Function: fork_analogdaq
 * ------------------------
 *  Launch analogdaq with arguments
 */
int fork_analogdaq(char **args){

    char path[LPATH];
    char mricomdir[LPATH] = {0};
    strcpy(mricomdir,getenv("MRICOMDIR"));
    int ret;
    pid_t p;
    snprintf(path, sizeof(path),"%s/%sanalogdaq",mricomdir,BIN_DIR);
    printf("path: %s\n",path);

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
        char *cmdargs[2];
        cmdargs[0] = path;

        //cmdargs[1] = "design"; // why do this again??
        //cmdargs[2] = args[2];
        cmdargs[1] = NULL;
        ret = execvp(path,cmdargs);
        if(ret < 0){
            perror("fork_analogdaq: execvp");
            exit(1);
        }
        return 0;
    }
}

int fork_ttlctrl(char **args){
    return 0;
}

/*
 * Function: mribg_status_check
 * ----------------------------
 *  Return -1 if mribg is set to automated mode
 */
//TODO use if things get more complicated
int mribg_status_check(){

    // mribg_status
    // 0 = manual
    // 1 = auto
    // 2 = auto and experiment running
    if(mribg_status == 0){

        return 0;
    }
}
