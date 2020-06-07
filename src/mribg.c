/*
 * mribg.c
 *
 * Background control program for mricom automation
 *
 *
 *
 * Managing a study
 *
 */

#include "mribg.h"

#define VERBOSE 1


#define STATUS_MANUAL 0
#define STATUS_AUTO_WAITING 1
#define STATUS_AUTO_RUNNING 2
/* status controls mribg accept/reject behaviour*/
int mribg_status = 0;
int is_analogdaq_on = 0;
struct study *study;

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
    mribg_status = gs->mribg_init_status;

    // init global study struct
    study = malloc(sizeof(struct study));
    memset(study,0,sizeof(struct study));
    study->seqnum = 0;
    strcpy(study->id,"NONE");

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
        ret = process_request(gs,buffer, msg_back);

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
 *      stop
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
 *      vnmrclient,start
 *      mricom,get,status
 *      mricom,set,status,1
 *      vnmrclient,set,study,s_2020052701
 *      vnmrclient,set,seqname,epip_01
 *      ttlctrl,stop
 *
 *
 * General rules
 * - 'start' only comes from vnmrclient or mricom
 * - 'start' should signal the start of a sequence
 * - 'start' accept/reject is decided based on sender and status
 * - 'set' is used to modify mribg global variables, eg: struct study, status
 * - 'set' can be called before 'start' by vnmrclient to signal for new sequence
 * - 'get' is used to ask for global variables to be sent over
 * - 'stop' is mainly sent by ttlctrl when sequence finishes OK
 *   TODO:
 * - 'abort' can be sent by vnmrclient on sequence abort 
 */

int process_request(struct gen_settings *gs,char *msg, char *msg_response){

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


    // -----------------------------------
    //              START
    // -----------------------------------
    if(strcmp(argv[1], "start")==0){
        
        // start from vmrclient
        // --------------------
        if(strcmp(argv[0],"vnmrclient")==0){
            // status check
            if(mribg_status_check(0) == 0){
                // fork ttlctrl from here
                fork_ttlctrl(NULL);
                mribg_status = STATUS_AUTO_RUNNING;
            } else {
                return -1;
            }

        }
        // start from mricom
        // -----------------
        else if(strcmp(argv[0],"mricom")==0){
        
            // status check
            if(mribg_status_check(1) == 0){
                ;

            } else {
                return -1;
            }
        }
        // neither 
        else {
            fprintf(stderr,"start can only be called from here\n");
            return -1;
        }
        // if checks passed start stim or daq process
        // ------------------------------------------
        if(strcmp(argv[2], "blockstim")==0){
            for(i=0;i<argc;i++){
                // blockstim doesnt need the first 2 argument
                strcpy(cmdargv[i],argv[i+2]);
            }
            fork_blockstim(cmdargv);
        }
        else if(strcmp(argv[2], "analogdaq")==0){
            if(is_analogdaq_on == 0){
                ret = fork_analogdaq(cmdargv);
                is_analogdaq_on = ret; // child pid is returned
            } else{
                fprintf(stderr, "analodag already running\n");
                return -1;
            }
        }
        else {
            fprintf(stderr,"action not supported\n");
            return -1;
        }
        return 1;
    }

    // -----------------------------------
    //              STOP
    // -----------------------------------

    
    if(strcmp(argv[1],"stop") == 0){
        // check if mribg is in auto mode

        // stop from ttlctrl, this mean sequence has finished properly
        // --------------------
        if(strcmp(argv[0],"ttlctrl")==0){
            ret = mribg_status_check(2);
            if(ret < 0){
                return -1;
            }
            // TODO data handling
            mribg_status = STATUS_AUTO_WAITING;
            datahandler(gs, study, "stop");
            return 1;
        }   
        // stop from mricom
        // --------------------
        else if(strcmp(argv[0],"mricom")==0){
            ;
            if(strcmp(argv[2], "blockstim")==0){
                //TODO simply interrupt blockstim??
                return 1;
            }
        }
    }

    // -----------------------------------
    //              SET
    // -----------------------------------
    if( argc == 4 && strcmp(argv[1],"set") == 0){
        // setting mribg_status from mricom
        if(strcmp(argv[0],"mricom")==0){

            if(strcmp(argv[2], "status")==0){

                ret = mribg_status_check(3);
                if(ret < 0){
                    return -1;
                }
                mribg_status = atoi(argv[3]);
                return 1;
            }
            // setting study struct, usually from vnmrclient
            // study id, eg: s_2020050401
            else if(strcmp(argv[2],"study_id")==0){
                strcpy(study->id, argv[3]);
                return 1;
            }
            // sequence fid directory name, eg: epip_hd
            else if(strcmp(argv[2],"study_sequence")==0){
                strcpy(study->sequence[study->seqnum], argv[3]);
                (study->seqnum)++;
                return 1;
            }
        }
        // setting study struct from vnmrclient
        else if(strcmp(argv[0],"vnmrclient")==0){
            // setting study struct, usually from vnmrclient
            // study id, eg: s_2020050401
            if(strcmp(argv[2],"study_id")==0){
                //TODO create new dir
                strcpy(study->id, argv[3]);
                create_study_dir(gs, study);
                return 1;
            }
            // sequence fid directory name, eg: epip_hd
            else if(strcmp(argv[2],"study_sequence")==0){
                strcpy(study->sequence[study->seqnum], argv[3]);
                (study->seqnum)++;
                return 1;
            }
        }
        // request anywhere else are rejected TODO mayzbe ttlctrl?
        else {
            return -1;
        }
            
    }

    // -----------------------------------
    //              GET
    // -----------------------------------
    if(argc == 3 && strcmp(argv[1],"get") == 0){
        if(strcmp(argv[2], "status")==0){
            memset(msg_response, 0, sizeof(char)*BUFS);
            snprintf(msg_response, BUFS, "%d",mribg_status);
            return 0;
        }
        else if(strcmp(argv[2], "study_id")==0){
            memset(msg_response, 0, sizeof(char)*BUFS);
            snprintf(msg_response, BUFS, "%s",study->id);
            return 0;
        }
        else if(strcmp(argv[2], "study_seqnum")==0){
            memset(msg_response, 0, sizeof(char)*BUFS);
            snprintf(msg_response, BUFS, "%d",study->seqnum);
            return 0;
        }
        else if(strcmp(argv[2], "study_sequence")==0){
            if(study->seqnum > 0){
                memset(msg_response, 0, sizeof(char)*BUFS);
                snprintf(msg_response, BUFS, "%s",study->sequence[study->seqnum-1]);
                return 0;
            } else {
                fprintf(stderr, "There was no sequence started yet\n");
                return -1;
            }
        }
    }

    //TODO
    // -----------------------------------
    //               ABORT
    // -----------------------------------
    if(strcmp(argv[1],"abort")){
        ;
    }
    // fin, reject for unkown reason if code gets here
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

    // TODO arguments not supported yet
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

/*
 * Function: fork_ttlctrl
 * ----------------------
 *  Launch ttlctrl in the background with arguments
 */ 
int fork_ttlctrl(char **args){

    char path[LPATH];
    char mricomdir[LPATH] = {0};
    int ret;

    strcpy(mricomdir,getenv("MRICOMDIR"));
    snprintf(path, sizeof(path),"%s/%sttlctrl",mricomdir,BIN_DIR);

    pid_t p;

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
        cmdargs[1] = NULL;
        ret = execvp(path,cmdargs);
        if(ret < 0){
            perror("fork_analogdaq: execvp");
            exit(1);
        }
        return 0;
    }
}

/*
 * Function: fork_mriarch
 * ----------------------
 * Launch mriarch for data management
 */
int fork_mriarch(char **args){
    return 0;
}

/*
 * Function: mribg_status_check
 * ----------------------------
 *  Return 0 if process_request is allowed to proceed, -1 othervise
 *
 *  The input 'state' represents the possible scenarios
 *  
 *  state
 *      0 - start request from vnmrclient
 *      1 - start request from mricom
 *      2 - stop request from ttlctrl at sequence end
 *      3 - setting status from mricom
 */
int mribg_status_check(int state){

    switch (state){

        case 0:
            if(mribg_status == STATUS_AUTO_WAITING){
                return 0;
            } else if(mribg_status == STATUS_AUTO_RUNNING){
                fprintf(stderr, "mribg start error: sequence already running");
                return -1;
            } else if(mribg_status == STATUS_MANUAL){
                fprintf(stderr, "mribg start error: set status to manual");
                return -1;
            }
            break;
        case 1:
            break;
        case 2:
            if(mribg_status == STATUS_AUTO_RUNNING){
                return 0;
            } else if(mribg_status == STATUS_AUTO_WAITING){
                fprintf(stderr, "error: wrong mribg_status");
                return -1;
            } else if(mribg_status == STATUS_MANUAL){
                fprintf(stderr, "error: wrong mribg_status");
                return -1;
            }
            break;

        case 3:
            //TODO
            if(mribg_status == STATUS_AUTO_RUNNING){
                return 0;
            } else {
                return 0;
            }
            break;
    }
}

/*
 * Function: datahandler
 * ---------------------
 */
int datahandler(struct gen_settings *gs, struct study *st, char *action){

    return 0;
}

/*
 * Function: create_study_dir
 * --------------------------
 */

void create_study_dir(struct gen_settings *gs, struct study *stud){

    struct stat st = {0};
    char path[LPATH*3] = {0};
    snprintf(path, sizeof(path), "%s/%s",gs->studies_dir, stud->id);
    if(stat(gs->studies_dir, &st) == -1){
        mkdir(gs->studies_dir, 0700);
    }
    if(stat(path, &st) == -1){
        mkdir(path, 0700);
    }

}

/*
 * Function: create_sequence_dir
 * -----------------------------
 */
void create_sequence_dir(struct gen_settings *gs, struct study *stud){

    struct stat st = {0};
    char path[LPATH*3] = {0};
    snprintf(path, sizeof(path), "%s/%s/%s",
            gs->studies_dir,stud->id, stud->sequence[stud->seqnum]);
    if(stat(path, &st) == -1){
        mkdir(path, 0700);
    }

}


