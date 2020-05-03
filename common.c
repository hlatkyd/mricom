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
        free(buf);
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
    printf("%s\n",line);
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
 * Function: sighandler
 * --------------------
 *  Catch interrupts (eg.: Ctrl-c) and finish log file before termination
 */
void sighandler(int signum){
    // accepts no argument, so set up mpid again
    struct mpid *mp;
    char path[] = MPROC_FILE;
    if(signum == 2){
        mp = malloc(sizeof(struct mpid));
        fill_mpid(mp);
        printf("pid, ppid %d, %d, %s\n",mp->pid, mp->ppid, mp->name);
        processctrl_add(path, mp, "INTRPT");
        exit(1);

    }

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
 * Function: parse_settings
 * ------------------------
 * reads settings file and fills daq_settings struct
 */
 //TODO make this obsolete
int parse_settings(struct gen_settings *settings,
                    struct dev_settings *devsettings){


    char settings_file[] = SETTINGS_FILE;
    FILE *fp;
    char line[128];
    char buf[128];
    char *token;
    int len;
    int i = 0; int j = 0;
    int nchan = NCHAN; // for comparing number of channel to channel names

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
        if(strcmp(token, "DAQ_FILE") == 0){
            token = strtok(NULL,"=");
            strcpy(settings->daq_file, token);
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
        /* kst2 settings */
        if(strcmp(line, "KST_SETTINGS") == 0){
            token = strtok(NULL,"=");
            strcpy(settings->kst_settings, token);
            continue;
        }
        if(strcmp(line, "CHANNELS") == 0){
            token = strtok(NULL,"=");
            // so far 'default' only...
            if(strcmp(token,"default") == 0){
                settings->channels = NCHAN;
            } else {
                printf("settings error, channels only 'default'\n");
                settings->channels = NCHAN;
            }
            continue;

        }
        if(strcmp(line, "CHANNEL_NAMES") == 0){
            i = 0;
            token = strtok(NULL,"=");
            strcpy(buf, token);
            token = strtok(buf, ",");
            while(token != NULL){
                strcpy(settings->channel_names[i], token);
                i++;
                token = strtok(NULL,",");
            }
            if(i != nchan){
                printf("warning: more channels than channel names\n");
            }
            i = 0; // reset to 0
            continue;
        }
        /* device settings */
        if(strcmp(line, "IS_ANALOG_DIFFERENTIAL") == 0){
            token = strtok(NULL,"=");
            devsettings->is_analog_differential = atoi(token);
            continue;
        }
        if(strcmp(line, "ANALOG_IN_SUBDEV") == 0){
            token = strtok(NULL,"=");
            devsettings->analog_in_subdev = atoi(token);
            continue;
        }
        if(strcmp(line, "STIM_TRIG_SUBDEV") == 0){
            token = strtok(NULL,"=");
            devsettings->stim_trig_subdev = atoi(token);
            continue;
        }
        if(strcmp(line, "STIM_TRIG_CHAN") == 0){
            token = strtok(NULL,"=");
            devsettings->stim_trig_chan = atoi(token);
            continue;
        }
        if(strcmp(line, "ANALOG_IN_CHAN") == 0){
            i=0;
            token = strtok(NULL,"=");
            strcpy(buf, token);
            token = strtok(buf, ",");
            while(token != NULL){
                devsettings->analog_in_chan[i] = (int)atoi(token);
                i++;
                token = strtok(NULL,",");
            }
            i = 0; // set 0 again, just to be sure
        }
    }
    return 0;
}
int parse_gen_settings(struct gen_settings *settings){

    char settings_file[] = SETTINGS_FILE;
    FILE *fp;
    char line[128];
    char buf[128];
    char tmpbuf[128] = {0};
    char *token;
    int len;
    int i = 0; int j = 0;
    int nchan = NCHAN; // for comparing number of channel to channel names

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

    char settings_file[] = SETTINGS_FILE;
    FILE *fp;
    char line[128];
    char buf[128];
    char *token;
    int len;
    int i = 0; int j = 0;
    int naichan = NAICHAN; // for comparing number of channel to channel names

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
        if(strcmp(line, "IS_ANALOG_DIFFERENTIAL") == 0){
            token = strtok(NULL,"=");
            ds->is_analog_differential = atoi(token);
            continue;
        }
        if(strcmp(line, "ANALOG_IN_SUBDEV") == 0){
            token = strtok(NULL,"=");
            ds->analog_in_subdev = atoi(token);
            continue;
        }
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
    }
    return 0;
}

/*
 * Function: fprint_common_header
 * ------------------------------
 * write timing, version, etc info into file common to tsv and meta
 */
int fprintf_common_header(FILE *fp, struct header *h, char **args){

    char line[64];
    char buf[64];
    int vmaj = VERSION_MAJOR;
    int vmin = VERSION_MINOR;
    if(fp == NULL){
        printf("fprint_header_common: file not open\n");
        exit(1);
    }
    gethrtime(buf, h->timestamp);
    fprintf(fp,"# cmd=%s args=%s,%s Mricom v%d.%d\n",
               h->proc, args[1], args[2], vmaj, vmin);
    fprintf(fp,"# timestamp=%s\n", buf);
    return 0;

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

