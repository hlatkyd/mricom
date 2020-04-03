/* parser.c
 * 
 * parse vnmrj procpar file
 *
 * parse settings file
 */

#include "mricom.h"

extern daq_settings *settings;

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

    char settings_file[] = SETTINGS_FILE;
    FILE *fp;
    char line[128];
    char *token;
    char *buf;
    int len;

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
        //remove newline
        len = strlen(line);
        if(line[len-1] == '\n')
            line[len-1] = 0;
        /* general settings */
        if(strncmp(line, "DEVICE", 6) == 0){
            token = strtok(line,"=");
            token = strtok(NULL,"=");
            strcpy(settings->device, token);
        }
        if(strncmp(line, "DAQ_FILE", 8) == 0){
            token = strtok(line,"=");
            token = strtok(NULL,"=");
            strcpy(settings->daq_file, token);
        }
        if(strncmp(line, "PROCPAR", 7) == 0){
            token = strtok(line,"=");
            token = strtok(NULL,"=");
            strcpy(settings->procpar, token);
        }
        if(strncmp(line, "EVENT_DIR", 9) == 0){
            token = strtok(line,"=");
            token = strtok(NULL,"=");
            strcpy(settings->event_dir, token);
        }
        if(strncmp(line, "RAMDISK", 7) == 0){
            token = strtok(line,"=");
            token = strtok(NULL,"=");
            strcpy(settings->ramdisk, token);
        }
        /* kst2 settings */
        if(strncmp(line, "KST_SETTINGS", 12) == 0){
            token = strtok(line,"=");
            token = strtok(NULL,"=");
            strcpy(settings->kst_settings, token);
        }
        if(strncmp(line, "CHANNELS", 8) == 0){
            token = strtok(line,"=");
            token = strtok(NULL,"=");
            settings->channels = atoi(token);
        }
        if(strncmp(line, "CHANNEL_NAMES", 8) == 0){
            token = strtok(line,"=");
            token = strtok(NULL,"=");
            strcpy(buf, token);
        }
    }
    return 0;
}