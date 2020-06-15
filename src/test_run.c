/*
 * test_run.c
 *
 * Simulates full automated run on a series of sequences. Mricom should be 
 * started as normal and mribg set to auto mode. 
 *
 * test_run plays the role of console host and the console as well.
 * On sequence start Vnmrj calls vnmrclient to set sequence and study name,
 * then another vnmrclient call with the argument 'start' signals to setup
 * stimulation and start the sequence. 
 *
 * the list of sequence names and associated stimulation is taken from the file
 * test_exp.txt. 
 *
 * Usage: simply launch mricom as usual, then ./test_run from the test dir in
 * another bash instance.
 */

#include "common.h"

#define EXPFILE "test_exp.txt"
#define N_SEQ_MAX 64
#define PL 128
#define N_ARGS 6
int parse_expfile(char *expfile, struct study *st, int *seqtime,
                  char **stim, char **design);
void printf_pars(struct study *st, int *seqtime, char **stim, char **design);

int run_vnmrclient(char **args);
int run_test_console(int len);

int main(int argc, char **argv){

    char exp_file[] = EXPFILE;
    int seqtime[N_SEQ_MAX] = {0};
    char **stim;
    char **design;
    char **args; // vnmrclient command line args
    struct study *st;
    int i, j;


    char vpath[LPATH*2]; // vnmrclient path
    snprintf(vpath, sizeof(vpath),"%s/bin/vnmrclient",getenv("MRICOMDIR"));

    st = malloc(sizeof(struct study));
    stim = malloc(sizeof(char*)*N_SEQ_MAX);
    for(i=0; i<N_SEQ_MAX; i++){
        stim[i] = malloc(sizeof(char)*PL);
    }
    design = malloc(sizeof(char*)*N_SEQ_MAX);
    for(i=0; i<N_SEQ_MAX; i++){
        design[i] = malloc(sizeof(char)*PL);
    }

    parse_expfile(exp_file, st, seqtime, stim, design);
    printf_pars(st, seqtime, stim, design);

    // set study id 
    args = malloc(sizeof(char*) * N_ARGS);
    for(j=0; j<N_ARGS; j++ ){
        args[j] = malloc(sizeof(char) * PL);
    }
    strcpy(args[0],vpath);
    strcpy(args[1],"set");
    strcpy(args[2],"study_id");
    strcpy(args[3], st->id);
    args[4] = NULL;
    run_vnmrclient(args);
    free(args);

    // start running mock sequences
    for(i=0; i < st->seqnum; i++){
        // redo malloc args
        
        args = malloc(sizeof(char*) * N_ARGS);
        for(j=0; j<N_ARGS; j++ ){
            args[j] = malloc(sizeof(char) * PL);
        }
        
        // setup args for vnmrclient
        // set study id first
        strcpy(args[0],vpath);
        strcpy(args[1],"set");
        strcpy(args[2],"study_sequence");
        strcpy(args[3], st->sequence[i]);
        args[4] = NULL;
        run_vnmrclient(args);
        free(args);

        if(strcmp(stim[i],"") == 0){
            args = malloc(sizeof(char*) * 3);
            for(j=0; j<3; j++ ){
                args[j] = malloc(sizeof(char) * PL);
            }
            strcpy(args[0],vpath);
            strcpy(args[1],"start");
            args[2] = NULL;
        } else {
            args = malloc(sizeof(char*) * N_ARGS);
            for(j=0; j<N_ARGS; j++ ){
                args[j] = malloc(sizeof(char) * PL);
            }
            strcpy(args[0],vpath);
            strcpy(args[1],"start");
            strcpy(args[2],stim[i]);
            strcpy(args[3],"design");
            strcpy(args[4],design[i]);
            args[5] = NULL;
            printf("args 2 %s\n",args[2]);
            printf("args 4 %s\n",args[4]);
            printf("args 0 %s\n",args[0]);
        }
        run_vnmrclient(args);

        run_test_console(seqtime[i]);

        free(args);
    }

    free(st);
    free(design);
    free(stim);

    return 0;
}

int run_test_console(int time){

    char path[LPATH*2];
    char **args;
    char str_time[16];
    int j;
    snprintf(str_time, sizeof(16), "%d",time);
    snprintf(path, sizeof(path),"%s/test/test_console",getenv("MRICOMDIR"));
    args = malloc(sizeof(char*) * 3);
    for(j=0; j<N_ARGS; j++ ){
        args[j] = malloc(sizeof(char) * PL);
    }
    strcpy(args[0],path);
    strcpy(args[1],str_time);
    args[2] = NULL;
    /*Spawn a child to run the program.*/
    pid_t pid=fork();
    if (pid==0) { /* child process */
        execv(args[0],args);
        exit(127); /* only if execv fails */
    }
    else { /* pid!=0; parent process */
        waitpid(pid,0,0); /* wait for child to exit */
    }
    return 0;
}
int run_vnmrclient(char **argv){

    /*Spawn a child to run the program.*/
    pid_t pid=fork();
    if (pid==0) { /* child process */
        execv(argv[0],argv);
        exit(127); /* only if execv fails */
    }
    else { /* pid!=0; parent process */
        waitpid(pid,0,0); /* wait for child to exit */
    }
    return 0;
}



int parse_expfile(char *expfile, struct study *st, int *seqtime,
                  char **stim, char **design){
   
    FILE *fp;
    char *line;
    char buf[128];
    size_t len = 0;
    ssize_t read;
    char *token;
    int tcount, lcount, i;
    const char d[] = ",";
    lcount = 0;
    i = 0;
    fp = fopen(expfile,"r");
    if(fp == NULL){
        perror("fopen");
        exit(1);
    }
    while((read = getline(&line, &len, fp)) != -1){
        strtok(line, "\n"); // remove newline from end
        tcount = 0;
        if(line[0]=='#' || line[0]==' ' || line[0]=='\n' || line[0]=='\t'){
            continue;
        } else {
            if(lcount == 0){
                strcpy(st->id,line);
                lcount++;
                continue;
            }
            token = strtok(line, d);
            while(token){
                switch(tcount){
                    case 0:
                        strcpy(st->sequence[i],token);
                        break;
                    case 1:
                        seqtime[i] = atoi(token);
                        break;
                    case 2:
                        strcpy(stim[i], token);
                        break;
                    case 3:
                        strcpy(design[i], token);
                        break;
                }
                token = strtok(NULL, d);
                tcount++;
            }
            i++;
        }
        lcount++;
    }
    st->seqnum = lcount-1;

    return 0; 
}

void printf_pars(struct study *st, int *seqtime, char **stim, char **design){

    int i;
    int n = st->seqnum;
    printf("Study ID: %s\n",st->id);
    printf("seqnum: %d\n\n",n);
    for(i=0; i<n; i++){

        printf("%s,%d,%s,%s\n",st->sequence[i],seqtime[i],stim[i],design[i]);
    }

}
