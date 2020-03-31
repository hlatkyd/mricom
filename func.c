/* func.c
 *
 * mricom user function definitions
 *
 * contents:
 *  - test functions
 *  - util daq functions
 *  - util shell functions
 *  - util init
 *  - util user functions
 *  - main user functions 
 */

#include "mricom.h"
#include "func.h"

extern processes *procpt;
extern acquisition_const *acqconst;
extern daq_data *data;
extern daq_settings *settings;

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
    char *kstpath[] = {"kst2",
                    "/home/david/dev/mricom/test/kst_test.kst",
                    NULL};
    pid_t pid;
    pid = fork();
    printf("hello forks!\n");

    if(pid == 0){

        // child
        //system("kst2");
        //execvp(kstpath[0], kstpath);
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

    double duration = 30.0; // duration of data generation
    int iter, max_iter;
    double time, itertime, timestep;
    char process_name[] = "test_datagenerator";
    FILE *fp;
    pid_t pid;

    timestep = 1 / acqconst->sampling_rate;
    time = 0.0;
    max_iter = 60;
    itertime = (double)NDATA/SAMPLING_RATE;
    /* fork and run */
    if(pid == 0){

        // child
        for(iter=0; iter<max_iter; iter++){
            time = time + itertime*iter;
            test_randfill_buffer(time);
            daq_save_buffer();
            sleep(1);
        }
        //TODO process_remove something
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
}
//TODO fix
void test_randfill_buffer(double start_time){

    return;
}
void test_system(){
    return;
}
/*-------------------------------------------------------------------*/
/*                    util  daq functions                            */
/*-------------------------------------------------------------------*/

/* Function: daq_start_acq()
 * --------------------------
 * Start acquisition on comedi device
 */
void daq_start_acq(){

    return;
}

/* Function: daq_init_kstfile
 * --------------------------
 * Creates daq file read by kst. Fills initial data points with zeros
 */
 //TODO fix data_buffer, make obsolete, REDO ALL
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
        //data->window[0][i] = -acqconst->time_window + timestep * i; 
    }
    for(i=0; i<acqconst->c_dwindow; i++){
        for(j=1; j<acqconst->chnum; j++){
          //  data->window[j][i] = 0.0; 
        }
    }
    for(i=0; i<acqconst->c_dbuffer; i++){
        for(j=0; j<acqconst->chnum; j++){
            //data_buffer[j][i] = 0.0; 
        }
    }

    /* start writing file */
    fp = fopen(filepath,"w");
    if(fp == NULL){
        //TODO check for this in init 
        printf("\nerror writing daq file on path %s\n",filepath);
        if(DEBUG > 0){
            printf("Hint: check permissions, mounts...\n");
        }
        printf("Exiting...\n");
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
            //fprintf(fp,"%lf%s",data->window[j][i],dlt);
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
            //fprintf(fp,"%lf%s",data_buffer[j][i],dlt);
        }
        fprintf(fp,"\n");
    }

    fclose(fp);

}
/* Function: daq_start_kst
 * -----------------------
 * Starts kst with standard acquisition settings
 */
void daq_start_kst(){

    char *kst_path;
    char *kst_settings_file;

    char process_name[] = "kst";
    //TODO make this better, use settings file
    char *kstpath[] = {"/usr/bin/kst2",
                       "/home/david/dev/mricom/mricomkst.kst",
                       NULL};
    pid_t pid;
    pid = fork();

    if(pid == 0){
        // child
        //system("kst2");
        execvp(kstpath[0], kstpath);
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
/*-------------------------------------------------------------------*/
/*                     util init functions                           */
/*-------------------------------------------------------------------*/
//TODO move this in another file
/* read settings.txt in parent directory and fill daq_settings struct*/
void parse_settings_file(){

    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int n; // for counting position of '=' in settings line

    fp = fopen("settings","r");
    if(fp == NULL){
        printf("error on opening 'mricom_settings.txt'\n");
        exit(EXIT_FAILURE);
    }

    while((read = getline(&line, &len, fp)) != -1){
        if(line[0] == '#'){
            continue; // leave out comments
        }
        printf("%s",line);

    }


    fclose(fp);
    free(line);
    return;
}
/* check for kst2 install */
int is_kst_accessible(){

    FILE *fp;
    char path[1024];
    char *s;
    fp = popen("which kst2","r");

    if(fp == NULL){
        printf("kst2 was not found in PATH.\n");
        return 1;
    }
    s = fgets(path, sizeof(path),fp);
    if(s == NULL){
        printf("kst2 was not found in PATH.\n");
        return 1;
    }
    if(DEBUG > 0){
        printf("kst2 found at %s",path);
    }
    return 0;
}
/* as title...*/
int is_nicard_accessible(){
    FILE *fp;
    char path[1024];
    int lines = 0;
    int mlines = 5; // max lines to display when reading comedi_board_info

    if(DEBUG > 0){
        printf("Checking comedi device...\n"); 
    }
    //system("comedi_board_info");
    fp = popen("comedi_board_info","r");
    if(fp == NULL){
        printf("command not found: 'comedi_board_info'");
        return 1;
    }

    while((fgets(path, sizeof(path),fp) != NULL) && (lines != mlines)){
        if(lines == 0){
            lines += 1;
            continue; // leave out the first line, no info there
        }
        printf("%s",path);
        lines += 1;
    }
    if(lines == mlines){
        if(DEBUG > 0){
            printf("Comedi device OK...\n"); 
        }
        return 0;
    } else {
        printf("Comedi device not found...\n");
        return 1;
    }
}
/* check for mounted ramdisk */
int is_ramdisk_accessible(){
    
    if(access(RAMDISK, W_OK) == 0){
        if(DEBUG > 0){
            printf("Ramdisk mounted at %s...\n",RAMDISK);
        }
        return 0;
    } else {
        return 1;
    }
}

/*-------------------------------------------------------------------*/
/*                     util user functions                           */
/*-------------------------------------------------------------------*/
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
