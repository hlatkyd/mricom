/* mricom:  Collect data with NI card via comedi and control MRI
 * 
 * mricom.c: responsible for the interactive shell and general setup
 *
 * shell code base: Stephen Brennan
 * https://github.com/brenns10/lsh
 *
 */
#include "mricom.h"
#include "common.h"
#include "socketcomm.h"
#include "func.h"

/* init device settings struct*/
struct gen_settings *gs;
struct dev_settings *ds;
struct processes *pr;
struct mpid *mmp;
int fd_read, fd_write; // file descriptors to communicate with mribg


//TODO review these, less is more
/* command names
 * 
 * should be the same order as builtin command pointer list */
char *builtin_str[] = {
    "exit",
    "help",
    "killp",
    "test",
    "start",
    "stop",
    "list",
    "clean",
    "update",
    "send"
};
/* functions for builtin commands*/
/* should be same oreder as builtin_str list names*/
int (*builtin_func[]) (int, char **) = {
    &sh_exit,
    &sh_help,
    &sh_killp,
    &sh_test,
    &sh_start,
    &sh_stop,
    &sh_list,
    &sh_clean,
    &sh_update,
    &sh_send

};
int sh_num_builtins(){
    return sizeof(builtin_str) / sizeof(char*);
}

/* builtin implementations */

/* Shell function: sh_exit
 * -----------------------
 * attempt to exit mricom and mribg gracefully, and clean everything
 */
 //TODO check for unintended stop ps -ef, or something
int sh_exit(int argc, char **args){
    
    exit_cleanup();
    return 0;
}
/* Shell function sh_help
 * ----------------------
 * prints available builtin commands
 */

int sh_help(int argc, char **args){
    int i, n;
    char *cmdname;
    cmdname = NULL;
    if(args[1] == NULL){
        printf("--- mricom v%d.%d ---\n\n",VERSION_MAJOR,VERSION_MINOR);
        printf("Available commands:\n");
        printf("===================\n");
        for(i = 0; i<sh_num_builtins(); i++){
            printf("  %s\n",builtin_str[i]);
        }
    }
    else{
        for(i=0; i<sh_num_builtins(); i++){
            if(strcmp(builtin_str[i],args[1])==0){
                cmdname = args[1];
                n = i;
            }
        }
        if(cmdname == NULL){
            printf("No such command as '%s'\n",args[1]);
            return 1;
        }
        printf("COMMAND: %s\n", cmdname);
        printf("---------------\n");
        switch (n)
        {
            case 0: // exit
                printf("exit mricom gracefully\n");
                break;
            case 1: // help
                printf("print available commands or get help for a command\n");
                break;
            case 2: // killp
                printf("kill mricom childprocess by id\n");
                break;
            case 3: // test
                printf("test everything?\n");
                break;
            case 4: // start
                printf("start data acquisition\n");
                break;
            case 5: // stop
                printf("stop data acquisition\n");
                break;
            case 6: // list
                printf("print settings struct\n");
                break;
            case 7: // clean
                printf("clean mproc.log file, only leave current processes\n");
                break;
            case 8: // update
                printf("get status update from mribg\n");
                break;
        }
        printf("\n");

    }
    return 1;
}
int sh_test(int argc, char **args){
    //TODO implement usage by specific arguments
    // start, stop, stim etc

    return 1;
}
int sh_killp(int argc, char **args){
    //TODO
    int procid;
    int i;
    char argin[10];
    if(argc != 2){
        fprintf(stderr, "killp: wrong arguments.Usage: 'killp [int process id]'\n");
    }
    procid = atoi(args[1]);
    // check if process really is running
    processctrl_get(gs->mpid_file, pr);
    for(i=0;i<pr->nproc;i++){
        if(pr->pid[i] == procid){

            printf("killing process: %s\n",pr->name[i]);
            kill(procid, SIGINT);
            processctrl_get(gs->mpid_file, pr);
            return 1;
        }
    }
    fprintf(stderr, "mricom process %d cannot be found\n",procid);
    return 1;
}
int sh_start(int argc, char **args){
    //TODO make arguments useful, eg kst start

    return 1;
}
int sh_stop(int argc, char **args){
    // TODO make arguments as well, eg kst stop
    
    int ret;
    if(argc == 2 && strcmp(args[1],"mribg") == 0){
        ret = stop_mribg(pr->bgpid);
        if(ret < 0){
            fprintf(stderr, "error stopping mribg\n");
            return 0;
        } else if(ret == 0){
            fprintf(stderr, "mricom: mribg stopped\n");
        }
        
    }
    return 1;
}
int sh_list(int argc, char **args){
    //TODO maybe arguments here as well such as list [args, eg set]??
    if(argc > 1){
        if(strcmp(args[1],"settings")==0){
            printf("\n");
            listsettings(gs);
            listdevsettings(ds);
            printf("\n");
            return 1;
        } else if (strcmp(args[1],"proc")==0){
            processctrl_get(gs->mpid_file, pr);
            listprocesses(pr);
            return 1;
        } else {
            printf("unknown argument %s\n",args[1]);
            return 1;
        }
    }
}
/*
 * Function: sh_clean
 * ------------------
 *  clean contents of mproc.log, so that only running processes are kept
 */
int sh_clean(int argc, char **args){
    
    int ret;
    processctrl_get(gs->mpid_file, pr);
    ret = processctrl_clean(gs, pr);
    if(ret == 0){
        printf(" cleaned mproc.log\n");
    }
    return 1;
}
/*
 * Function: sh_update
 * -------------------
 *  update experiment satus from mribg
 */
int sh_update(int argc, char **args){

    return 1;
}

/*
 * Function: sh_send
 * -------------------
 *  Send message to mribg
 */
#define BUFSIZE 256
int sh_send(int argc, char **args){

    char buf[BUFSIZE] = {0};
    // input check
    if(argc != 2){
        fprintf(stderr, " Incorrect number of arguments. Usage: send [string msg]");
        return 1;
    }
    // TODO allow multiple args, and concat them to proper form
    
    strcpy(buf, args[1]);
    send_mribg(buf);
    return 1;
}

/*-----------------------------------------------------------*/
/*                    OPAQUE PARTS                           */
/*-----------------------------------------------------------*/
/* Function: init
 * ---------------
 * print name, version, initialize process struct, init command history
 */
void init(){

    int is_dev_ok, ret;
    char mricomdir[128];

    printf("\n-----------------------------------------------\n");
    printf("mricom v%d.%d",VERSION_MAJOR,VERSION_MINOR);
    printf(" - MRI control software using comedi\n");
    printf("-----------------------------------------------\n");

    // check env
    if(getenv("MRICOMDIR") == NULL){
        fprintf(stderr, "environment variable MRICOMDIR not set, exiting\n");
        exit(1);
    }

    // malloc for settings
    ds = malloc(sizeof(struct dev_settings));
    gs = malloc(sizeof(struct gen_settings));
    pr = malloc(sizeof(struct processes));
    memset(ds, 0, sizeof(struct dev_settings));
    memset(gs, 0, sizeof(struct gen_settings));
    memset(pr, 0, sizeof(struct processes));
    pr->mainpid = getpid();
    parse_gen_settings(gs);
    parse_dev_settings(ds);
    // clear contents of mproc.log
    if(CLEAR_MPROC_LOG == 1){
        FILE *fp;
        fp = fopen(gs->mpid_file,"w");
        fclose(fp);
    }
    mmp = malloc(sizeof(struct mpid));
    memset(mmp, 0, sizeof(struct mpid));
    fill_mpid(mmp);
    processctrl_add(gs->mpid_file, mmp, "START");
    /* check for kst2 install and ramdisk mount and device */

    if(DEBUG > 0){
        printf("\nmricom startup check...\n");
    }
    if(is_ramdisk_accessible(gs) != 0){
        printf("\n\tWarning: ramdisk mount point not found!\n");
    }
    if(is_kst_accessible(gs) != 0){
        printf("\n\tWarning: installed kst2 not found!\n");
    }
    if(is_nicard_accessible(gs) != 0){
        printf("\n\tWarning: NI card not found!\n");
        is_dev_ok = 0;
    } else {
        is_dev_ok = 1;
    }

    //is_accessible(kstpath);

    //TODO do this properly
    //launch_process("procmonitor");

}

void init_msg(){

    printf("\nType 'help' to list available commands.\n");
    printf("-----------------------------------------------\n");
}

/*
 * Function: cleanup
 * -----------------
 * write mpid log, stop mribg
 */
void exit_cleanup(){

    char procname[16] = {0};
    //TODO wrong pid somehow
    getname(procname, pr->bgpid);
    if(strncmp(procname,"mribg",5)==0){
        kill(pr->bgpid, SIGTERM);
        //printf("kill bgpid: %d\n",pr->bgpid);
    }
    processctrl_add(gs->mpid_file, mmp, "STOP");

    free(gs);
    free(ds);
    free(pr);
}

/* Function: shell_parse_cmd
 * -------------------------
 * tokenizes the user input
 *
 * line: user input of the command and arguments separated by spaces
 *
 * returns: list of strings containing the command and arguments
 */

char **shell_parse_cmd(char *line){

    int bufsize = MAX_ARG_LENGTH;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens){
        fprintf(stderr, "parse_cmd: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line,ARG_DELIM);
    while(token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += MAX_ARG_LENGTH;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens) {
                fprintf(stderr, "parse_cmd: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, ARG_DELIM);
    }
    tokens[position] = NULL;
    free(token);
    return tokens;
}

/* Function: shell_read_command
 * ----------------------------
 * reads user input into command buffer using readline lib
 *
 * returns: command string
 */

//TODO tab completion in readline
char *shell_read_cmd(){

    int i;
    char *buffer; // no malloc needed if using readline lib
    while(1){
        if((buffer = readline(">>> ")) != NULL){
            if(strlen(buffer) > 0){
                add_history(buffer);
                return buffer;
            }
        }
        else{
            fprintf(stderr, "shell_read_line: allocation error\n");
            exit(EXIT_FAILURE);
        }
    }

}

/* Function: shell_get_argc
 * ----------------------
 * counts argument strings 
 */
int shell_get_argc(char **args){

    int i = 0;
    while(1){
        if(args[i] != NULL){
            i++;
            continue;
        } else
            break;
    }
    return i;
}
/* Function: shell_launch
 * ----------------------
 * launches original shell commmands 
 */
int shell_launch(int argc, char **args){
    
    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0){
        // child process
        if(execvp(args[0], args) == -1){
            printf("%s: No such command\n",args[0]);
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0){
        //error forking
        perror("shell_launch");
    }
    else {
        // parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}
/* Function: shell_execute
 * -----------------------
 * executes builtin command if name matches, othervise execute system command
 */
int shell_execute(int argc, char **args){
    int i;

    if(args[0] == NULL) {
        //empty command
        return 1;
    }
    // prohibited shell commands
    if(strcmp(args[0],"kill") == 0){
        printf("did you mean 'killp [id]?'\n");
        return 1;
    }
    if(strcmp(args[0],"rm") == 0){
        printf("no 'rm' here!\n");
        return 1;
    }
    // starting builtins first if found
    for(i=0; i<sh_num_builtins();i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(argc, args);
        }
    }
    // if not a builtin, then lauch as shell command
    return shell_launch(argc, args);
}

/*
 * Function: mribg_launch
 * ----------------------
 * Start background program for handling automation.
 *
 * mribg executable should be in MRICOMDIR environment variable
 * Uses fork for launching in background and socket for communication
 *
 *
 */
#define MAXBUF 80
int mribg_launch(){

    char bginfifo[LPATH]={0};
    char bgoutfifo[LPATH]={0};
    char mricomdir[LPATH] = {0};

    pid_t p;

    strcpy(mricomdir,getenv("MRICOMDIR"));

    p = fork();
    if(p < 0){
        fprintf(stderr, "mribg_launch: fork failed");
        exit(1);
    // parent process, mricom
    } else if(p > 0) {
    
        return p;

    // child process, mribg
    } else {
        
        // launch mribg
        char path[LPATH] = {0};
        char *args[2];
        snprintf(path, sizeof(path),"%s/mribg",mricomdir);
        args[0] = path;
        args[1] = NULL;
        execvp(path,args);
        return 0;
    }
}



int main(int argc_cmd, char *argv_cmd[]){


    //shell_loop();
    char *line;
    char **args; 
    int res;
    int argc;
    int ret;

    init();

    // launch mribg backgorund process
    ret = mribg_launch();
    if(ret < 0){
        fprintf(stderr, "cannot launch mribg\n");
    } else{
        pr->bgpid = ret;
    }

    init_msg();

    signal(SIGINT,sighandler);
    
    do {

        line = shell_read_cmd();

        // TODO put process monitor cycle here????

        args = shell_parse_cmd(line);

        argc = shell_get_argc(args);

        res = shell_execute(argc, args);

        free(line);
        free(args);

    } while(res != 0);

    return EXIT_SUCCESS;
}
