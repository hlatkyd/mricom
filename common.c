/* common.c
 * 
 * Common function definitions include varoius parsers and
 * mricom specific process management
 * 
 */
#include "common.h"

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
int parse_settings(struct daq_settings *settings,
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
/* Function: parse_blockstim_conf
 * -------------------------------
 * Fill struct blockstim_settings from config file
 *
 * Args:
 */

int parse_blockstim_conf(struct blockstim_settings *bs, char *conffile, char *n){

    #define N_PARS 8            // number of params set in conf file

    FILE *fp;
    char line[128];
    char buf[128];
    char *token;
    double temp;
    int len;
    int i = 0; int j = 0; int count = 0;
    int start = 0;

    fp = fopen(conffile, "r");
    if(fp == NULL){
        printf("\nparser_blockstim_conf: could not open file %s\n",conffile);
        exit(1);
    }
    while(fgets(line, 128, fp)){
        // ignore whitespace and comments
        if(line[0] == '\n' || line[0] == '\t'
           || line[0] == '#' || line[0] == ' '){
            continue;
        }
        count++;
        //remove whitespace
        remove_spaces(line);
        //remove newline
        len = strlen(line);
        if(line[len-1] == '\n')
            line[len-1] = '\0';
        /* general settings */
        
        token = strtok(line,"=");
        // find first line referring to design 'n'
        if(strcmp(token,"DESIGN") == 0){
            token = strtok(NULL,"=");
            if(strcmp(token,n) == 0){
                // found the line, save number
                start = count;
                continue; 
            } else {
                continue;
            }
        } else if (start != 0 && count > start && count < start + N_PARS + 1){
            // read params here
            if (strcmp(token,"SUBDEV") == 0){
                token = strtok(NULL,"=");
                bs->subdev = atoi(token);
                continue;
            }
            if (strcmp(token,"CHAN") == 0){
                token = strtok(NULL,"=");
                bs->chan = atoi(token);
                continue;
            }
            if (strcmp(token,"START_DELAY") == 0){
                token = strtok(NULL,"=");
                sscanf(token, "%lf", &temp);
                bs->start_delay = temp;
                continue;
            }
            if (strcmp(token,"ON_TIME") == 0){
                token = strtok(NULL,"=");
                sscanf(token, "%lf", &temp);
                bs->on_time = temp;
                continue;
            }
            if (strcmp(token,"OFF_TIME") == 0){
                token = strtok(NULL,"=");
                sscanf(token, "%lf",&temp);
                bs->off_time = temp;
                continue;
            }
            if (strcmp(token,"TTL_USECW") == 0){
                token = strtok(NULL,"=");
                sscanf(token, "%lf", &temp);
                bs->ttl_usecw = temp;
                continue;
            }
            if (strcmp(token,"TTL_FREQ") == 0){
                token = strtok(NULL,"=");
                sscanf(token, "%lf", &temp);
                bs->ttl_freq = temp;
                continue;
            }
            if (strcmp(token,"N_BLOCKS") == 0){
                token = strtok(NULL,"=");
                bs->n_blocks = atoi(token);
                continue;
            }
        } else {
            continue;
        }
    }

    return 0;
}
/*
 * Function: fprint_common_header
 * ------------------------------
 * write timing, version, etc info into file common to tsv and meta
 */
int fprintf_header_common(FILE *fp, struct header_common *h){

    char line[64];
    char buf[64];
    int vmaj = VERSION_MAJOR;
    int vmin = VERSION_MINOR;
    if(fp == NULL){
        printf("fprint_header_common: file not open\n");
        exit(1);
    }
    gethrtime(buf, h->start_time);
    fprintf(fp,"# %s, v%d.%d\n",h->process_name, vmaj, vmin);
    fprintf(fp,"# %s\n", buf);
    return 0;

}

/* Function: getppname
 * -------------------------
 * Find parent process name and put into string pointer input
 */
int getppname(char *name){

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
    strcpy(name, pname);
    return 0;
}

/*
 * Function gethrtime
 * ------------------
 * Copy timeval into human readable string buffer
 */
void gethrtime(char *buf, struct timeval tv){

    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64];

    gettimeofday(&tv, NULL);
    nowtime = tv.tv_sec;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
    snprintf(buf, sizeof buf, "%s.%06ld", tmbuf, tv.tv_usec);

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

