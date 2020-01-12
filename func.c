/* func.c
 *
 * mricom user function definitions
 */

#include "mricom.h"
#include "func.h"

extern processes *procpt;
extern history *cmdhist;
extern acquisition_const *acqconst;
extern double **data_window;
extern double **data_buffer;

/*-------------------------------------------------------------------*/
/*                           test functions                          */
/*-------------------------------------------------------------------*/
void test_print(char **args){
    printf("test_print says hello\n");
    return;
}
void test_fork(){

    int i;
    int procnum;
    char testname[] = "test_process";
    char kstpath[] = "/usr/bin/kst2";
    char *args[] = {"/home/david/dev/mricom/test/kst_test.kst", NULL};
    pid_t pid;
    pid = fork();
    printf("hello forks!\n");

    if(pid == 0){

        // child
        //system("kst2");
        execvp(kstpath, args);
        //perror("execv");
        return;
    }
    else if(pid < 0){
        //error forking
        perror("error forking");
    }
    else {
        // parent
        process_add(pid, testname);
    }
    return;

}
/* fork a process and start random data generation */
void test_generate_loop(){

    int chnum = acqconst->chnum;
    int npoints = acqconst->c_dbuffer;
    int srate = SAMPLING_RATE;
    char process_name[] = "test_datagenerator";
    FILE *fp;
    pid_t pid;

    /* setup loop */
    double **data;
    data = (double**)malloc(srate * chnum * sizeof(double));

    /* fork and run */
    if(pid == 0){

        // child
        //system("kst2");
        //test_randfill_buffer()
        //perror("execv");
        return;
    }
    else if(pid < 0){
        //error forking
        perror("error forking");
    }
    else {
        // parent
        process_add(pid, process_name);
    }
    return;

    free(data);
}
/* generate one batch of test data into data_buffer*/
void test_randfill_buffer(double start_time){

    /* generate times*/
    int i, j;
    double time, timestep;

    time = start_time;
    timestep = 1 / acqconst->sampling_rate; 
    for(i=0; i<acqconst->c_dbuffer; i++){
        time = timestep * i;
        data_buffer[0][i] = time;
    }
    /* generate random data*/
    for(i=0; i<acqconst->c_dbuffer; i++){
        for(j=1; j<acqconst->chnum; j++){
            data_buffer[j][i] = (double)rand()/(RAND_MAX);
        }
    }
   
    return;
}
void test_system(){
    /*
    char kstpath[] = "/usr/bin/kst2";
    char *args[] = {"/home/david/dev/mricom/kst_test.kst", NULL};
    execvp(kstpath, args);
    perror("execv");
    */
    return;
}
/*-------------------------------------------------------------------*/
/*                          daq functions                            */
/*-------------------------------------------------------------------*/

/* Function: daq_init_kstfile
 * --------------------------
 * Creates daq file read by kst. Fills initial data points with zeros
 */
void daq_init_kstfile(){

    int nchan = NACHAN + NDCHAN;
    int samples = SAMPLING_RATE * TIME_WINDOW;
    int i,j;
    double timestep;
    char *dlt = DELIMITER;
    char *filepath = DAQ_FILE;
    FILE *fp;
    /* prepare data */
    for(i=0; i<acqconst->c_dwindow;i++){
        timestep = acqconst->time_window / acqconst->c_dwindow;
        data_window[0][i] = -acqconst->time_window + timestep * i; 
    }
    for(i=0; i<acqconst->c_dwindow; i++){
        for(j=1; j<acqconst->chnum; j++){
            data_window[j][i] = 0.0; 
        }
    }
    for(i=0; i<acqconst->c_dbuffer; i++){
        for(j=0; j<acqconst->chnum; j++){
            data_buffer[j][i] = 0.0; 
        }
    }

    /* start writing file */
    fp = fopen(filepath,"w");
    if(fp == NULL){
        printf("error writing daq file on path %s\n",filepath);
        exit(EXIT_FAILURE);
    }
    
    /* write header */
    for(i=0; i<acqconst->chnum; i++){
        fprintf(fp,"%s%s",acqconst->chname[i],dlt);
    }
    fprintf(fp,"\n");
    /* write data */

    for(i=0; i<acqconst->c_dwindow; i++){
        for(j=0; j<acqconst->chnum; j++){
            fprintf(fp,"%lf%s",data_window[j][i],dlt);
        }
        fprintf(fp,"\n");
    }

    fclose(fp);

}
/* Function: daq_update_window
 * ---------------------------
 * Shifts data window, and appends buffered data to the end
 */
void daq_update_window(){

}

/* Function: daq_save_buffer
 * ---------------------------
 * Append buffer data to end of daq file
 */
void daq_save_buffer(){

    FILE *fp;
    char *dlt = DELIMITER;
    int i, j;

    fp = fopen(acqconst->acqfile, "a+");
    if(fp == NULL){
        printf("error writing daq file on path %s\n",acqconst->acqfile);
        exit(EXIT_FAILURE);
    }
    /* write data */
    for(i=0; i<acqconst->c_dbuffer; i++){
        for(j=0; j<acqconst->chnum; j++){
            fprintf(fp,"%lf%s",data_buffer[j][i],dlt);
        }
        fprintf(fp,"\n");
    }

    fclose(fp);

}
/*-------------------------------------------------------------------*/
/*                     util shell functions                          */
/*-------------------------------------------------------------------*/
void addpid(int pid){
    printf("adding pid: %d\n",pid);
    return;
}
void process_add(int pid, char *procname){
    
    int n;
    //printf("adding process '%s' with pid %d\n",procname, pid); 
    n = procpt->nproc;
    procpt->procid[n] = pid;
    strcpy(procpt->name[n], procname);
    procpt->nproc = n + 1;
}
void process_remove(int pid){

    char name;
    int n = procpt->nproc;
    int index;
    int i;
    /* find the index of process with procid*/
    for(i=0; i<=n; i++){
        if(pid == procpt->procid[i]){
            index = i;
        }
    }
    if(index != n){
        /* remove from name list and rearrange list*/
        for(i=index; i<n; i++){
            strcpy(procpt->name[i],procpt->name[i+1]);
            procpt->procid[i] = procpt->procid[i+1];
        }
        procpt->nproc = n - 1;
    }
    else{
        strcpy(procpt->name[index],"");
        procpt->procid[index] = 0;
        procpt->nproc = n - 1;
    }
}
void append_to_history(char *buffer){

    int num;
    num = cmdhist->n;
    strcpy(cmdhist->cmd[num], buffer);
    cmdhist->n = num + 1;
    return;
}

/*-------------------------------------------------------------------*/
/*                     util user functions                           */
/*-------------------------------------------------------------------*/
void getproc(){
    return;
}
/* list command history */
void listh(){
    
    int i;
    printf("Command history: \n");
    for(i=0;i<cmdhist->n; i++){
        printf("%s\n",cmdhist->cmd[i]);
    }
    return;
}
/* list current processes */
void listp(){

    int i;
    int n = procpt->nproc;
    printf("Mricom processes running:\n");
    if(n!=0){
        for (i = 0; i < n; i++){
            printf("name: %s, pid: %d\n",
                            procpt->name[i], procpt->procid[i]);
        }
    }
    return;
}
/* kill specified process */
void killp(int procid){
    printf("killing process: %d ... ",procid);
    /* search through local process tree for check*/
    kill(procid, SIGTERM);
    process_remove(procid);
    printf("done\n");
    return;
}
/*-------------------------------------------------------------------*/
/*                     main user functions                           */
/*-------------------------------------------------------------------*/
void start(){

    return;
}

void stop(){

    return;
}
/* Function: reset
 * ---------------
 * Kills internal processes, reinitializes acquisition settings
 */
void reset(){
    int i, j;
    printf("Resetting acquisition... ");
    for(i=procpt->nproc; i>0; i--){
        killp(procpt->procid[i-1]);
    }
    daq_init_kstfile();
    printf("done\n");
    return;
}
