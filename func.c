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
#include "common.h"
#include "func.h"
#include "mricom.h"
#include "comedifunc.h"


extern processes *procpt;
extern daq_data *data;
extern daq_settings *settings;
extern dev_settings *devsettings;

/*-------------------------------------------------------------------*/
/*                           test functions                          */
/*-------------------------------------------------------------------*/
void test_print(char **args){

    printf("test_print says: %s\n",args[1]);
    return;
}
void testproc(){

    printf("starting testproc...\n");
    sleep(5);
    printf("testproc ended...\n");
    return;
}
//TODO make obsolete
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
//TODO
// not as important as daq_analog_start or something
/*
 * Function: test_daq_simulate
 * ------------------------------
 * generate random data and append to daq_file
 * use the standard daq settings defined at compile time or settings file
 */
 //TODO shell command be 'test start' and 'test stop'
void test_daq_simulate(){

    int i = 0;
    double time, itertime, timestep;
    char process_name[] = "test_datagenerator";
    FILE *fp;
    pid_t pid;

    //timestep = 1 / acqconst->sampling_rate;
    time = 0.0;
    //itertime = 0.5;
    itertime = 1;

    /* fork and run */
    pid = fork();

    if(pid == 0){

        // child
        while(1){
            //TODO make a timer function here
            test_rand_membuf(time);
            daq_append_membuf();
            daq_update_window();
            daq_update_kstfile();
            time += itertime;
            i++;
            //usleep(itertime * 1000000);
            sleep(itertime);
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
/*
 * Function test_randfill_buffer
 * -----------------------------
 * Fill data->membuf with random values and progress time
 * 
 * INPUT:
 *      double start_time -- time at the start of buffer in sec
 *                           with usec accuracy
 */
 //TODO test execution time of this
void test_rand_membuf(double start_time){

    int i, j;
    int naichan = NAICHAN;
    int naochan = NAOCHAN;
    int ndichan = NDICHAN;
    int ndochan = NDOCHAN;
    int nchan = NCHAN;
    int samples = SAMPLING_RATE;
    double timestep = (double) 1 / SAMPLING_RATE;
    double t;
    srand(time(NULL));
    for(j=0;j<samples;j++){
        // fill columns
        for(i=0;i<nchan;i++){
            // time
            t = start_time + timestep * j;
            if(i == 0){
                data->membuf[0][j] = t;
            } else if(i>0 && i < naichan + 1){
                // analog input channels
                // fill resp with sin x
                if(i == 1){
                    data->membuf[i][j] = sin(2*M_PI*t);
                } else {
                    data->membuf[i][j] = (double)rand()/RAND_MAX;
                }
            } else if(i > naichan +1  && i < naichan + naochan + 1){
                // analog output
                data->membuf[i][j] = (double)rand()/RAND_MAX;
            } else if(i>naichan + naochan + 1 &&
                      i < naichan + naochan + ndichan + 1){
                // digital input
                if (j * 2 / samples == 1){
                    data->membuf[i][j] = 1;
                } else {
                    data->membuf[i][j] = 0;
                }
            } else if(i>naichan + naochan + 1 &&
                      i < naichan + naochan + ndichan + 1){
                //digital oputput
                data->membuf[i][j] = rand() % 2;
            }
        }
    }
    return;
}
void test_system(){
    return;
}
/*-------------------------------------------------------------------*/
/*                    util  daq functions                            */
/*-------------------------------------------------------------------*/

/*
 * Function: daq_timer_start
 * -------------------------
 * Start software timer used to merge analog and digital data. Also
 * used for digital stimulation timing. Called on init as well.
 */

void daq_timer_start(){

    gettimeofday(&data->clock,NULL);
    return;
}

/*
 * Function: daq_timer_elapsed
 * -------------------------
 * elapsed time in sec, with usec accuracy from program start
 *
 * Return
 *      double [elapsed time]
 */

double daq_timer_elapsed(){

    struct timeval cur,start;
    start = data->clock;
    gettimeofday(&cur,NULL);
    double elapsed = (cur.tv_sec - start.tv_sec);
    elapsed += (cur.tv_usec - start.tv_usec) / 1000000.0; //usec to sec
    return elapsed;

}
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
 *
 * kst data file format
 *
 * # TIME   CHANNEL_1  CHANNEL_2    .... 
 * time_1   data_1     data_2       ..
 *
 * channels go from analog input_1-n, analog output, digital in, digital out etc
 */

void daq_init_kstfile(){

    //TODO backup, check for orig
    int nchan = NCHAN;
    int samples = SAMPLING_RATE * TIME_WINDOW;
    double time_window = (double) TIME_WINDOW;
    double timestep = (double) 1 / SAMPLING_RATE;
    char *dlt = DELIMITER;
    FILE *fp;
    int i,j; // i is channel; j is timepoint index
    /* prepare data */
    for(i=0; i<nchan;i++){
        for(j=0; j<samples; j++){
            // [channel][datapoint]
            if(i == 0){
                data->window[0][j] = -time_window + timestep * (j+1); 
            } else {
                data->window[i][j] = 0.0; 
            }
        }
    }

    //start writing file /
    fp = fopen(settings->daq_file,"w");
    if(fp == NULL){
        printf("\nerror writing daq file on path %s\n",settings->daq_file);
        if(DEBUG > 0){
            printf("Hint: check permissions, mounts...\n");
        }
        printf("Exiting...\n");
        exit(EXIT_FAILURE);
    }
    
    // write header
    fprintf_header(fp);
    // write data

    for(j=0; j<samples; j++){
        for(i=0; i<nchan; i++){
            fprintf(fp,"%.*lf%s",settings->precision,data->window[i][j],dlt);
        }
        fprintf(fp,"\n");
    }
    fclose(fp);

}
/* Function: daq_update_file
 * ---------------------------
 * save whole data window into file
 */
void daq_update_kstfile(){

    FILE *fp;
    int samples = SAMPLING_RATE;
    int nchan = NCHAN;
    int window = TIME_WINDOW * SAMPLING_RATE;
    char *dlt = DELIMITER;
    int i, j;

    fp = fopen(settings->kst_file,"w+");
    fprintf_header(fp);
    for(j=0; j<window; j++){
        for(i=0; i<nchan; i++){
            fprintf(fp,"%.*lf%s",settings->precision,data->window[i][j],dlt);
        }
        fprintf(fp,"\n");
    }
    fclose(fp);
}
/* Function: daq_update_window
 * ---------------------------
 * Shifts data window backwards, and appends 'membuf' data to the end
 */
void daq_update_window(){

    int i, j;
    int nchan = NCHAN;
    int samples = SAMPLING_RATE;
    int window = SAMPLING_RATE * TIME_WINDOW;

    for(j=0;j<window;j++){
        for(i=0;i<nchan;i++){
            // shift back
            if(j<window-samples){
                data->window[i][j] = data->window[i][j+samples];
            }
            // fill end with membuf data
            if(j>window-samples){
                data->window[i][j] = data->membuf[i][j - (window-samples)];
            }
        }
    }

}

/* Function: daq_save_buffer
 * ---------------------------
 * Append full buffer data (analog and digital) to end of daq file
 */
void daq_append_membuf(){

    FILE *fp;
    char *dlt = DELIMITER;
    int samples = SAMPLING_RATE;
    int nchan = NCHAN;
    int i, j;
    fp = settings->fp_daq;
    /*
    fp = fopen(settings->daq_file, "a");
    if(fp == NULL){
        printf("error writing daq file on path %s\n",settings->daq_file);
        exit(EXIT_FAILURE);
    }
    */
    /* write data */
    for(j=0; j<samples; j++){
        for(i=0; i<nchan; i++){
            fprintf(fp,"%.*lf%s",settings->precision,data->membuf[i][j],dlt);
        }
        fprintf(fp,"\n");
    }

    //fclose(fp);

}
/* Function: daq_launch_kst
 * -----------------------
 * Starts kst with standard acquisition settings
 */
void daq_launch_kst(){

    char *kst_path = settings->kst_path;
    char *kst_settings_file = settings->kst_settings;
    char process_name[] = "kst"; // local process name
    struct stat s= {0};
    // check if kst settings is in current dir
    if(settings->is_kst_on == 1){
        printf(" daq_launch_kst : a kst instance is already started\n");
        return;
    }
    if(!(stat(settings->kst_settings,&s))){
        if(ENOENT == errno)
            perror("can't find kst settings file in current dir\n");
    }
    char *kstcall[3];
    kstcall[0] = settings->kst_path;
    kstcall[1] = settings->kst_settings;
    kstcall[2] = NULL;

    pid_t pid;
    pid = fork();

    if(pid == 0){
        // child
        execvp(kstcall[0], kstcall);
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
        settings->is_kst_on = 1;
    }
    return;
}
/*-------------------------------------------------------------------*/
/*                     util shell functions                          */
/*-------------------------------------------------------------------*/

/*
 * Function: addpid
 * -----------------
 */
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
    printf("n: %d\n",n);
    /* find the index of process with procid*/
    for(i=0; i<=n; i++){
        printf("pid : %d, procid %d\n",pid,procpt->procid[i]);
        if(pid == procpt->procid[i]){
            index = i;
        }
    }
    if(DEBUG > 1){
        printf("removing process %d, index %d\n",pid,index);
    }
    
    if(strcmp(procpt->name[index],"kst") == 0)
        settings->is_kst_on = 0;
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

/* Function: is_kst_accessible
 * ---------------------------
 *
 * look for kst2 in PATH
 */
int is_kst_accessible(){

    FILE *fp;
    char path[1024];
    char *s, *pos;
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
    // strip eol
    if((pos = strchr(path, '\n')) != NULL){
        *pos = '\0';
    } else{
        printf("oops, something wrong in 'is_kst_accessible'\n");
    }
    strcpy(settings->kst_path, path);
    return 0;
}
/* Function: is_nicard_accessible
 * ------------------------------
 *
 * calls comedi_board_info, checks if output corresponds with card
 * TODO check for card version
 */

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
/* Function: is_nicard_accessible
 * ------------------------------
 *
 * checks if ramdisk is correctly mounted on path specified in settings
 */
int is_ramdisk_accessible(){
    
    if(access(settings->ramdisk, W_OK) == 0){
        if(DEBUG > 0){
            printf("Ramdisk mounted at %s...\n",settings->ramdisk);
        }
        return 0;
    } else {
        if(DEBUG > 0){
            printf("Ramdisk not found at %s...\n",settings->ramdisk);
        }
        return 1;
    }
}

/*
 * Function: fprintf_header
 * ------------------------
 *  write data file header
 */

int fprintf_header(FILE *fp){
    // TODO write additional info here, study number etc
    int nchan = NCHAN;
    char *dlt = DELIMITER;
    int i;

    // check if file open
    if(fp == NULL){
        perror("Failed to write header: ");
        return 1;
    }
    for(i=0; i<nchan; i++){
        fprintf(fp,"%s%s",settings->channel_names[i],dlt);
    }
    fprintf(fp,"\n");
    return 0;
}

/*
 * Function: procmonitor
 * ---------------------
 * checks for zombie processes coming from mricom and removes them
 * cleans local process list
 */

//TODO might not be the best way...
void procmonitor(){

    FILE *stream;
    char path[128];
    char *token;
    int i;
    int status;
    int sleeptime = 1000000; // in microsec
    pid_t pid, wpid;

    while(1){

        usleep(sleeptime);
        //TODO use fork and exec instead of popen
        //stream = popen("ps -ef | grep mricom | grep '<defunct>'","r");
        pid = fork();

        //child, call exec here
        if(pid == 0){
        
            sleep(1);
            exit(1);
        } else if(pid < 0){
            perror("procmonitor fork error: ");
        } else {
            //parent should wait for child
            listp();
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
}

/*
        if(stream == NULL){
            printf(" procmonitor: failed popen\n");
            usleep(sleeptime);
            continue;
        }
        while(fgets(path, sizeof(path),stream) != NULL){
            // if no 'grep mricom' in path only
            if(strstr(path," grep mricom ") == NULL){
                token = strtok(path,"\t");
                for(i=0;i<4;i++){
                    token = strtok(NULL, "\t");
                    printf("%s\n",token);
                }
                
            }
        }

        usleep(sleeptime);
    }
    printf("out of loop!? \n");
}
*/

/*
 * Function launch_process
 * -----------------------
 * fork and launch a background process.
 *
 * INPUT: process_name
 */

void launch_process(char *process_name){
    /*possible process names are  hardcoded, take care*/

    int procnum;
    int status;
    int retval;
    pid_t pid, cpid, wpid;

    pid = fork();
    if(pid == 0){
        // child

        // check for available process names
        if(strcmp(process_name,"procmonitor") == 0){
            procmonitor();

        } else if(strcmp(process_name, "stimtest") == 0){

            printf("testing digital trigger output...\n");
            comedi_digital_trig("events/testevent.evt");

        } else if(strcmp(process_name, "testproc") == 0){

            testproc();

        } else if(strcmp(process_name, "daq") == 0){

            comedi_start_analog_acq();

        } else {
            printf("cannot launch process '%s'\n",process_name);
        }
        exit(1);
    }
    else if(pid < 0){
        //error forking
        perror("error forking");
    }
    else {
        // parent

        process_add(pid, process_name);

        if(strcmp(process_name,"procmonitor") == 0){
            if(DEBUG > 0){
                printf(" starting 'procmonitor'\n");
            }
        }

        /*
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        */
    }
    return;

}
/*-------------------------------------------------------------------*/
/*                     util user functions                           */
/*-------------------------------------------------------------------*/

/*
 * Function: gettime
 * -----------------
 * print elapsed time from start of daq clock
 */

void gettime(){

    return;
}

/*
 * Function: listdevsettings
 * -----------------
 * list device settings acquired from settings file
 */

void listdevsettings(){

    int i = 0;
    printf("\ndevsettings struct:\n");
    printf("---------------------\n");
    printf("is_analog_differential: %d\n",
                devsettings->is_analog_differential);
    printf("analog_in_subdev: %d\n",devsettings->analog_in_subdev);
    printf("analog_in_chan: ");
    for(i;i<4;i++){
        printf("%d,",devsettings->analog_in_chan[i]);
    }
    printf("\nstim_trig_subdev: %d\n",devsettings->stim_trig_subdev);
    printf("stim_trig_chan: %d\n",devsettings->stim_trig_chan);
}
/*
 * Function: lsitsettings
 * -----------------
 * list settings acquired from settings file
 */

void listsettings(){

    int i = 0;
    // have to do it manually..
    printf("daq_settings struct:\n");
    printf("--------------------\n");
    printf("device : %s\n",settings->device);
    printf("daq_file : %s\n",settings->daq_file);
    printf("kst_file : %s\n",settings->kst_file);
    printf("precision: %d\n",settings->precision);
    printf("ramdisk: %s\n",settings->ramdisk);
    printf("procpar: %s\n",settings->procpar);
    printf("event_dir: %s\n",settings->event_dir);
    printf("kst_settings: %s\n",settings->kst_settings);
    printf("kst_path: %s\n",settings->kst_path);
    printf("channels: %d\n",settings->channels);
    printf("channel_names: ");
    for(i= 0; i<settings->channels;i++){
        printf("%s,",settings->channel_names[i]);
    }
    printf("\n");
    printf("is_daq_on: %d\n",settings->is_daq_on);
    printf("is_kst_on: %d\n",settings->is_kst_on);

    
}
/*
 * Function: catdata
 * -----------------
 * list contents of data file, same as 'cat [path_to_daq_datafile]'
 */

void catdata(){

}

/*
 * Function: listp
 * ---------------
 * list current child processes of mricom
 */

void listp(){

    int i;
    int n = procpt->nproc;
    printf("Number of mricom processes running: %d\n",n);
    printf(" main process: %d\n",procpt->mainpid);
    if(n!=0){
        for (i = 0; i < n; i++){
            printf(" child name: %s, pid: %d\n",
                            procpt->name[i], procpt->procid[i]);
        }
    }
    return;
}
/*
 * Function: killp
 * ---------------
 * kill child processes of mricom corresponding to process id
 * usage eg.:
 *   killp 24685
 *
 * input:
 *      procid
 */
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

    daq_launch_kst();
    daq_timer_start();
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

/*
 * Function: stimtest
 * ------------------
 * Run a brief digital trigger series
 *
 * Input: t -- stimulus length in seconds
 */

void stimtest(int t){

    //TODO timing not implemented yet

    launch_process("stimtest");
}
