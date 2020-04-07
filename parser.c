/* parser.c
 * 
 * parse vnmrj procpar file
 *
 * parse settings file
 */

#include "mricom.h"

extern daq_settings *settings;
extern dev_settings *devsettings;

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
int parse_settings(){

    void remove_spaces(char* s) {
        const char* d = s;
        do {
            while (*d == ' ') {
                ++d;
            }
        } while (*s++ = *d++);
    }

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
    }
    return 0;
}
