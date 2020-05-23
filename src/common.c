/* common.c
 * 
 * Common function definitions include varoius parsers and
 * mricom specific process management
 * 
 */
#include "common.h"
#define VERBOSE_PROCESSCTRL 1
#define LLENGTH 64 // mpid file line length
/* Function: fill_mpid
 * -------------------
 * fill mpid struct with pid, ppid, name, num from /proc
 */

void fill_mpid(struct mpid *mp){

    FILE *fp;
    //struct timeval tv;
    char path[32];
    char pidstr[8];
    char *buf = NULL;
    size_t len = 0;
    int i;
    gettimeofday(&(mp->tv), NULL);
    // get pid and parent pid
    mp->pid = getpid();
    mp->ppid = getppid();
    // get names by rading /proc/[pid]/comm
    pid_t pid;
    for(i=0;i<2;i++){
        if(i==0)
            pid = mp->pid;
        else
            pid = mp->ppid;
        sprintf(pidstr, "%d",pid);
        strcpy(path, "/proc/");
        strcat(path, pidstr);
        strcat(path, "/comm");
        //printf("path here : %s\n",path);
        fp = fopen(path,"r");
        if(fp == NULL){
            perror("fill_mpid");
            exit(1);
        }
        getline(&buf, &len, fp);
        //remove newline
        buf[strlen(buf)-1]='\0';
        if(i==0)
            strcpy(mp->name, buf);
        else
            strcpy(mp->pname, buf);
        fclose(fp);
        //TODO why segfault with this???
        //free(buf);
    }
}

/*
 * Function: processctrl_add
 * -------------------------
 * add process id to local pid file, return 0 on success
 *
 */
int processctrl_add(char *path, struct mpid *mp, char *status){
    
    int ret;
    char buf[64];
    char line[128];
    char *d = DELIMITER;
    struct timeval tv;
    int fd;
    FILE *fp;
    gettimeofday(&tv, NULL);
    gethrtime(buf, tv);
    if(strcmp(status,"START") == 0 || strcmp(status,"STOP") == 0 ||
            strcmp(status, "INTRPT") == 0){
        ;
    } else{
        printf("processctrl_add: wrong status input\n");
        exit(1);
    }
    fd = open(path, O_RDWR | O_APPEND);
    if(fd < 0){
        perror("processctrl_add");
        exit(1);
    }
    fp = fdopen(fd, "a+");
    // make line 
    sprintf(line,"%s%s%d%s%d%s%s%s%s%s%s",
            status,d,mp->pid,d,mp->ppid,d,mp->name,d,mp->pname,d,buf);
    // lock file access
    ret = flock(fd,LOCK_SH);
    if(fd < 0){
        perror("processctrl_add");
        exit(1);
    }
    // open file stream
    fprintf(fp, "%s\n",line);
    fclose(fp);
    // unlock file access
    ret = flock(fd,LOCK_UN);
    if(fd < 0){
        perror("processctrl_add");
        exit(1);
    }
    ret = close(fd);
    if(fd < 0){
        perror("processctrl_add");
        exit(1);
    }
    return 0;
}

/*
 * Function processctrl_get
 * ------------------------
 *  Parse process log file mproc.log and fill running process struct
 *  
 */
#define N_MAX 128
#define LLENGTH 64
int processctrl_get(char *path, struct processes *p){

    FILE *fp;
    char filebuf[N_MAX][LLENGTH];
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int i,j,k, n, col;
    int n_remove;
    int remove_list[N_MAX];

    int start_list[N_MAX], stop_list[N_MAX], intrpt_list[N_MAX];
    int running_list[N_MAX];

    char lstatus[N_MAX][8];
    char *d = DELIMITER;
    char *token;
    int tmp_pid;

    // start_list, stop_list, intrpt_list elements are either 0 or 1 so init 0
    memset(start_list, 0, N_MAX * sizeof(int));
    memset(stop_list, 0, N_MAX * sizeof(int));
    memset(intrpt_list, 0, N_MAX * sizeof(int));
    memset(running_list, 0, N_MAX * sizeof(int));
    fp = fopen(path,"r");
    if(fp == NULL){
        printf("could not open %s\n",path);
        return 1;
    }
    i = 0;
    // read into buffer
    while (read = getline(&line, &len, fp) != -1){
        if(strncmp(line, "#",1) == 0){
            continue;
        } else {
            //replace EOL with nullbyte
            line[strlen(line)-1] = '\0';
            strcpy(filebuf[i], line);
            // sort lines starting with START, STOP, INTRPT
            if(strncmp(line, "START",5)==0){
                start_list[i] = 1;    
            } else if(strncmp(line, "STOP",4)==0){
                stop_list[i] = 1;    
            } else if(strncmp(line, "INTRPT",6)==0){
                intrpt_list[i] = 1;    
            } else {
                fprintf(stderr, "processctrl_get: wrong line\n");
            }

        }
        i++;

    }
    // init running_list as start_list, remove elements later
    for(i=0;i<N_MAX;i++){
        running_list[i] = start_list[i];
    }
    // for 'START' lines check if there is a 'STOP' or 'INTRPT'
    for(i=0; i < N_MAX; i++){
        if(start_list[i] == 1){
            // check pid
            strcpy(line, filebuf[i]);
            token = strtok(line,d);
            tmp_pid = atoi(strtok(NULL,d));
            // check if same pid was stopped or interrupted
            for(j=0;j<N_MAX;j++){
                if((stop_list[j] == 1) || (intrpt_list[j] == 1)){
                    strcpy(line, filebuf[j]);
                    token = strtok(line,d);
                    if(atoi(strtok(NULL,d)) == tmp_pid){
                        running_list[i] = 0;
                    }
                    
                }
            }
        }
    }
    // tokenize line where 'running_list' is 1
    i=0;
    p->nproc = 0;
    for(j=0; j<N_MAX; j++){
        if(running_list[j] == 1){
            strcpy(line, filebuf[j]);
            if(strncmp(line,"START",5)==0){
                token = strtok(line,d);
                col = 0;
                while(token != NULL){
                    switch(col){
                        case 0 :
                            strcpy(lstatus[i],token);
                            break;
                        case 1 :
                            p->pid[i]=atoi(token);
                            break;
                        case 2 :
                            p->ppid[i]=atoi(token);
                            break;
                        case 3 :
                            strcpy(p->name[i],token);
                            break;
                        case 4 :
                            strcpy(p->pname[i],token);
                            break;
                        case 5 :
                            strcpy(p->timestamp[i],token);
                            break;
                    }
                    token = strtok(NULL, d);
                    col++;
                }
            }
            i++;
            p->nproc++;
        }

    }
    return 0;
}
/*
 * Function: sighandler
 * --------------------
 *  Catch interrupts (eg.: Ctrl-c) and finish log file before termination
 */
void sighandler(int signum){
    // accepts no argument, so set up mpid again
    struct mpid *mp;
    char c = '\0';
    char path[LPATH] = {0};

    strcpy(path, getenv("MRICOMDIR"));
    snprintf(path, sizeof(path),"%s/%s",getenv("MRICOMDIR"),MPROC_FILE);
    if(signum == SIGINT){
        mp = malloc(sizeof(struct mpid));
        memset(mp, 0, sizeof(struct mpid));
        fill_mpid(mp);
        // if interrupting mricom shell, kill mribg as well
        // TODO seems to exit anyway
        if(strcmp(mp->name, "mricom")==0){
           ; 
        }
        processctrl_add(path, mp, "INTRPT");
        fprintf(stderr,"%s exiting...\n",mp->name);
        free(mp);
        exit(1);
    }
    if(signum == SIGTERM){
        mp = malloc(sizeof(struct mpid));
        memset(mp, 0, sizeof(struct mpid));
        fill_mpid(mp);
        processctrl_add(path, mp, "INTRPT");
        fprintf(stderr,"%s exiting...\n",mp->name);
        free(mp);
        exit(1);
    }
}
/*
 * Function: processctrl_clean()
 * -----------------------------
 * Clear contents of mproc.log file, keep only currently running processes
 */
int processctrl_clean(struct gen_settings *gs, struct processes *pr){

    FILE *fp;
    int fd;
    int i, ret;
    char line[64];
    char *d = DELIMITER;
    //fd = open(gs->mpid_file);
    fd = open(gs->mpid_file, O_RDWR);
    if(fd < 0){
        perror("processctrl_clean");
        exit(1);
    }
    fp = fopen(gs->mpid_file,"w");
    ret = flock(fd,LOCK_SH);
    if(fd < 0){
        perror("processctrl_clean");
        exit(1);
    }
    for(i=0;i<pr->nproc;i++){
        snprintf(line, 64, "START%s%d%s%d%s%s%s%s%s%s\n", d, pr->pid[i], d, 
            pr->ppid[i], d, pr->name[i], d, pr->pname[i], d, pr->timestamp[i]);
        fprintf(fp,"%s",line);
    }
    fclose(fp);
    // unlock file access
    ret = flock(fd,LOCK_UN);
    if(fd < 0){
        perror("processctrl_clean");
        exit(1);
    }
    close(fd);
    return 0;
}

/*
 * Function: processctrl_archive
 * -----------------------------
 * Copy mproc.log contents with timestamp  into archive file
 */
//TODO
int processctrl_archive(char *path, char *archive){

    
    return 0;
}

/*
 * Function: parse_procpar
 * -----------------------
 */

int parse_procpar(){

}

/*
 * Function: search_procpar
 * -------------------------
 * search procpar file for specified parameter and finds its value
 * works only with string valued parameters eg.: comment, mricomcmd 
 *
 * input: 
 *      char *parameter_name 
 *      char *command 
 */
int search_procpar(char *parameter_name, char *command){

    // TODO get these from settings file
    char procpar[] = "procpar"; // procpar path
    char *parname = "comment "; // procpar parameter to search for
    char line[128]; // max procpar line length
    char cmd[64]; // string value of procpar parameter 'mricomcmd'
    int i,j,k,n = 0;

    int bool_found = 0;
    FILE *fp;

    fp = fopen(procpar,"r");
    if(fp == NULL){
        printf("cannot access file");
        exit(EXIT_FAILURE);
    }   
    // read only while parameter is not found
    while(fgets(line, 128, fp)){
        if (i > n && n != 0){
            break;
        }   
        if(strncmp(line, parname, strlen(parname)) == 0){
            n = i + 1;
        }   
        // this is the value line
        if(i == n && i != 0){
            while(line[j] != '\n'){
                if(j > 1 && line[j] != '"'){
                    cmd[k] = line[j];
                    k++;
                }
                j++;
            }   
        }   
        i++;
    }
    printf("comment value: %s\n",cmd);
    return 0;

}
/*
 * Function: parse_gen_settings
 * ------------------------
 * reads settings file and fills gen_settings struct
 */
int parse_gen_settings(struct gen_settings *settings){

    char settings_file[128] = {0} ;
    char mricomdir[128];
    FILE *fp;
    char line[128];
    char buf[128];
    char tmpbuf[128] = {0};
    char *token;
    int len;
    int i = 0; int j = 0;
    int nchan = NCHAN; // for comparing number of channel to channel names

    //check if MRICOMDIR exists as env TODO
    strcpy(mricomdir,getenv("MRICOMDIR"));
    strcat(settings_file, mricomdir);
    strcat(settings_file, "/");
    strcat(settings_file, SETTINGS_FILE);

    //set memory to zero
    memset(settings, 0, sizeof(*settings));
    fp = fopen(settings_file, "r");
    if(fp == NULL){
        printf("\nerror: could not open file 'settings'\n");
        printf("quitting ...\n");
        exit(EXIT_FAILURE);
    }
    while(fgets(line, 128, fp)){
        // ignore whitespace and comments
        if(line[0] == '\n' || line[0] == '\t'
           || line[0] == '#' || line[0] == ' '){
            continue;
        }
        //remove whitespace
        remove_spaces(line);
        //remove newline
        len = strlen(line);
        if(line[len-1] == '\n')
            line[len-1] = '\0';
        /* general settings */
        
        token = strtok(line,"=");

        if(strcmp(token,"DEVICE") == 0){
            token = strtok(NULL,"=");
            strcpy(settings->device, token);
            continue;
        }
        if(strcmp(token,"WORKDIR") == 0){
            token = strtok(NULL,"=");
            strcpy(settings->workdir, token);
            continue;
        }
        if(strcmp(token, "DAQ_FILE") == 0){
            token = strtok(NULL,"=");
            strcpy(settings->daq_file, token);
            continue;
        }
        if(strcmp(token, "PID_FILE") == 0){
            token = strtok(NULL,"=");
            strcpy(tmpbuf, token);
            continue;
        }
        if(strcmp(token, "KST_FILE") == 0){
            token = strtok(NULL,"=");
            strcpy(settings->kst_file, token);
            continue;
        }
        if(strcmp(token, "PRECISION") == 0){
            token = strtok(NULL,"=");
            settings->precision = atoi(token);
            continue;
        }
        if(strcmp(line, "PROCPAR") == 0){
            token = strtok(NULL,"=");
            strcpy(settings->procpar, token);
            continue;
        }
        if(strcmp(line, "EVENT_DIR") == 0){
            token = strtok(NULL,"=");
            strcpy(settings->event_dir, token);
            continue;
        }
        if(strcmp(line, "RAMDISK") == 0){
            token = strtok(NULL,"=");
            strcpy(settings->ramdisk, token);
            continue;
        }
    }
    // additional formatting
    strcat(settings->mpid_file,settings->workdir);
    strcat(settings->mpid_file,"/");
    strcat(settings->mpid_file, tmpbuf);
    return 0;
}
int parse_dev_settings(struct dev_settings *ds){

    char settings_file[128] = {0};
    char mricomdir[128];
    FILE *fp;
    char line[128];
    char buf[128];
    char *token;
    int len;
    int i = 0; int j = 0;
    int naichan = NAICHAN; // for comparing number of channel to channel names

    strcpy(mricomdir,getenv("MRICOMDIR"));
    strcat(settings_file, mricomdir);
    strcat(settings_file, "/");
    strcat(settings_file, SETTINGS_FILE);

    fp = fopen(settings_file, "r");
    if(fp == NULL){
        printf("\nerror: could not open file 'settings'\n");
        printf("quitting ...\n");
        exit(EXIT_FAILURE);
    }
    while(fgets(line, 128, fp)){
        // ignore whitespace and comments
        if(line[0] == '\n' || line[0] == '\t'
           || line[0] == '#' || line[0] == ' '){
            continue;
        }
        //remove whitespace
        remove_spaces(line);
        //remove newline
        len = strlen(line);
        if(line[len-1] == '\n')
            line[len-1] = '\0';
        
        token = strtok(line,"=");

        if(strcmp(token,"DEVPATH") == 0){
            token = strtok(NULL,"=");
            strcpy(ds->devpath, token);
            continue;
        }
        // analog input
        // -------------------------------------------
        if(strcmp(line, "IS_ANALOG_DIFFERENTIAL") == 0){
            token = strtok(NULL,"=");
            ds->is_analog_differential = atoi(token);
            continue;
        }
        if(strcmp(line, "ANALOG_SAMPLING_RATE") == 0){
            token = strtok(NULL,"=");
            ds->analog_sampling_rate = atoi(token);
            continue;
        }
        if(strcmp(line, "ANALOG_IN_SUBDEV") == 0){
            token = strtok(NULL,"=");
            ds->analog_in_subdev = atoi(token);
            continue;
        }
        if(strcmp(line, "ANALOG_CH_NAMES") == 0){
            i = 0;
            token = strtok(NULL,"=");
            strcpy(buf, token);
            token = strtok(buf, ",");
            while(token != NULL){
                strcpy(ds->analog_ch_names[i], token);
                i++;
                token = strtok(NULL,",");
            }
            i = 0; // reset to 0
            continue;
        }
        if(strcmp(line, "ANALOG_IN_CHAN") == 0){
            i=0;
            token = strtok(NULL,"=");
            strcpy(buf, token);
            token = strtok(buf, ",");
            while(token != NULL){
                ds->analog_in_chan[i] = (int)atoi(token);
                i++;
                token = strtok(NULL,",");
            }
            //TODO check if number is same as NAICHAN
            i = 0; // set 0 again, just to be sure
        }
        // digital stim
        // -----------------------------------------
        if(strcmp(line, "STIM_TRIG_SUBDEV") == 0){
            token = strtok(NULL,"=");
            ds->stim_trig_subdev = atoi(token);
            continue;
        }
        if(strcmp(line, "STIM_TRIG_CHAN") == 0){
            token = strtok(NULL,"=");
            ds->stim_trig_chan = atoi(token);
            continue;
        }
        // ttlctrl
        //----------------------------------------------
        if(strcmp(line, "TTLCTRL_SUBDEV") == 0){
            token = strtok(NULL,"=");
            ds->ttlctrl_subdev = atoi(token);
            continue;
        }
        if(strcmp(line, "TTLCTRL_OUT_CHAN") == 0){
            token = strtok(NULL,"=");
            ds->ttlctrl_out_chan = atoi(token);
            continue;
        }
        if(strcmp(line, "TTLCTRL_IN_CHANLIST") == 0){
            i=0;
            token = strtok(NULL,"=");
            strcpy(buf, token);
            token = strtok(buf, ",");
            while(token != NULL){
                ds->ttlctrl_in_chanlist[i] = (int)atoi(token);
                i++;
                token = strtok(NULL,",");
            }
            i = 0; // set 0 again, just to be sure
        }
    }
    return 0;
}

/*
 * Function: fprint_common_header
 * ------------------------------
 * write timing, version, etc info into file common to tsv and meta
 */
int fprintf_common_header(FILE *fp, struct header *h, int argc, char **args){

    char line[64];
    char buf[64];
    int vmaj = VERSION_MAJOR;
    int vmin = VERSION_MINOR;
    int i;
    if(fp == NULL){
        printf("fprint_header_common: file not open\n");
        exit(1);
    }
    gethrtime(buf, h->timestamp);
    fprintf(fp,"# cmd=%s args=",h->proc);
    if(argc==1){
        fprintf(fp,"NULL");
    } else {
        for(i=1;i<argc;i++){
            if(i==1)
                fprintf(fp,"%s",args[i]);
            else
            fprintf(fp,",%s",args[i]);
        }
    }
    fprintf(fp," Mricom v%d.%d\n",vmaj,vmin);
    fprintf(fp,"# timestamp=%s\n", buf);
    return 0;

}


/*
 * Function fprintf_times_meta
 * ---------------------------
 * log times struct in a common format in metadata files
 */

void fprintf_times_meta(FILE *fp, struct times *t){

    char *buf;
    buf = malloc(sizeof(char)*64); // for human readable time

    fprintf(fp, "\n%% TIMING\n");
    gethrtime(buf, t->start);
    fprintf(fp, "start=%s\n",buf);
    gethrtime(buf, t->action);
    fprintf(fp, "action=%s\n",buf);
    gethrtime(buf, t->stop);
    fprintf(fp, "stop=%s\n",buf);
    free(buf);

}



/*
 * Function: compare_common_header
 * --------------------------------
 * Return 1 if the 2-line headers are the same in the 2 files (.tsv and .meta)
 * 
 */
int compare_common_header(char *file1, char *file2){

    return 0;

}
/*
 * Function: getname
 * -----------------
 * get process name from process pid
 */
void getname(char *name, int pid){

    FILE *fp;
    char *procname = NULL;
    char path[32];
    char pidstr[8];
    size_t len = 0; 
    sprintf(pidstr, "%d",pid);
    strcpy(path, "/proc/");
    strcat(path, pidstr);
    strcat(path, "/comm");
    fp = fopen(path,"r");
    if(fp == NULL){
        fprintf(stderr, "getname: cannot open file %s\n",path);
        exit(1);
    }
    getline(&procname, &len, fp);
    strcpy(name, procname);
    free(procname);
    fclose(fp);
}

/* Function: getppname
 * -------------------------
 * Find parent process name and put into string pointer input
 */
void getppname(char *name){

    pid_t pid; 
    FILE *fp;
    char path[32];
    char pidstr[8];
    char *pname = NULL;
    size_t len = 0;
    pid = getppid();
    //itoa(pid,pidstr,10);
    sprintf(pidstr, "%d",pid);
    strcpy(path, "/proc/");
    strcat(path, pidstr);
    strcat(path, "/comm");
    //printf("path here : %s\n",path);
    fp = fopen(path,"r");
    if(fp == NULL){
        perror("getppname");
        exit(1);
    }
    getline(&pname, &len, fp);
    //TODO strip newline from end
    strcpy(name, pname);
    free(pname);
    fclose(fp);
}
/* Function: getcmdline
 * -------------------------
 * Find process invoking command and put into string pointer input
 */
 //TODO dont use this
void getcmdline(char *cmdl){

    pid_t pid; 
    //FILE *fp;
    int fd; // file descriptor
    char path[32];
    char pidstr[8];
    //char *buf = NULL;
    char *buf;
    buf = malloc(sizeof(char)*64);
    size_t len = 0;
    pid = getpid();
    //itoa(pid,pidstr,10);
    sprintf(pidstr, "%d",pid);
    strcpy(path, "/proc/");
    strcat(path, pidstr);
    strcat(path, "/cmdline");
    //printf("path here : %s\n",path);
    fd = open(path,O_RDONLY);
    if(fd == -1){
        perror("getcmdline");
        exit(1);
    }
    //getline(&buf, &len, fp);

    read(fd, buf, 64);
    strcpy(cmdl, buf);
    free(buf);
    close(fd);
}

/*
 * Function gethrtime
 * ------------------
 * Copy timeval into human readable string buffer
 * example: 2020-04-28 20:07:34.992715
 */
void gethrtime(char *outbuf, struct timeval tv){

    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64];
    char buf[64];

    memset(outbuf, 0, sizeof((*outbuf)));
    nowtime = tv.tv_sec;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
    snprintf(buf, sizeof buf, "%s.%06ld", tmbuf, tv.tv_usec);
    strcpy(outbuf, buf);

}
/*
 * Function clockhrtime
 * ------------------
 * Same as above, but with clock_gettime struct
 * Copy timeval into human readable string buffer
 * example: 2020-04-28 20:07:34.992715
 */
void getclockhrtime(char *outbuf, struct timespec tv){

    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64];
    char buf[64];

    memset(outbuf, 0, sizeof((*outbuf)));
    nowtime = tv.tv_sec;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
    snprintf(buf, sizeof buf, "%s.%09ld", tmbuf, tv.tv_nsec);
    strcpy(outbuf, buf);

}
/*
 * Function: getusecdelay
 * ----------------------
 * Calculate current difference in microsec from input time
 */
int getusecdelay(struct timeval tv1){

    struct timeval tv2;
    int time;
    int mic;
    double mega = 1000000;
    gettimeofday(&tv2,NULL);
    time = (tv2.tv_sec - tv1.tv_sec) * mega;
    mic = (tv2.tv_usec - tv1.tv_usec);
    mic += time;
    return mic;
}
/*
 * Function: clockusecdelay
 * ----------------------
 * Calculate current difference in microsec from input time
 */
int clockusecdelay(struct timespec tv1){

    struct timespec tv2;
    int time;
    int mic;
    double giga = 1000000000;
    clock_gettime(CLOCK_MONOTONIC,&tv2);
    time = (tv2.tv_sec - tv1.tv_sec) * giga;
    mic = (tv2.tv_nsec - tv1.tv_nsec);
    mic += time;
    return mic;
}
/*
 * Function: getsecdiff
 * ----------------------
 * Calculate difference in seconds (double) between two timepoints
 */
double getsecdiff(struct timeval tv1, struct timeval tv2){

    double diff;
    diff = tv2.tv_sec - tv1.tv_sec;
    diff += (double) (tv2.tv_usec - tv1.tv_usec) / 1000000.0;
    return diff;
}

/* Function: remove_spaces
 * -----------------------
 * remove whitespace from a line in config file
 */
void remove_spaces(char* s) {
    const char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

