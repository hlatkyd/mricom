/* func.c
 *
 * mricom user function definitions
 *
 */
#include "common.h"
#include "func.h"
#include "mricom.h"

/*-------------------------------------------------------------------*/
/*                     util shell functions                          */
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
/*                     util init functions                           */
/*-------------------------------------------------------------------*/

/* Function: is_kst_accessible
 * ---------------------------
 *
 * look for kst2 in PATH
 */
int is_kst_accessible(struct gen_settings *settings){

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
    return 0;
}
/* Function: is_nicard_accessible
 * ------------------------------
 *
 * calls comedi_board_info, checks if output corresponds with card
 * TODO check for card version
 */

int is_nicard_accessible(struct gen_settings *settings){
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
/* Function: is_ramdisk_accessible
 * ------------------------------
 *
 * checks if ramdisk is correctly mounted on path specified in settings
 */
int is_ramdisk_accessible(struct gen_settings *settings){
    
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

/*-------------------------------------------------------------------*/
/*                     util user functions                           */
/*-------------------------------------------------------------------*/

/*
 * Function: listdevsettings
 * -----------------
 * list device settings acquired from settings file
 */

void listdevsettings(struct dev_settings *devsettings){

    int i = 0;
    printf("\nstruct devsettings:\n");
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
 * Function: listsettings
 * -----------------
 * list settings acquired from settings file
 */

void listsettings(struct gen_settings *settings){

    int i = 0;
    // have to do it manually..
    printf("struct gen_settings:\n");
    printf("--------------------\n");
    printf("device : %s\n",settings->device);
    printf("daq_file : %s\n",settings->daq_file);
    printf("mpid_file : %s\n",settings->mpid_file);
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
 * Function: listprocesses
 * -----------------------
 * list running processes from mpid.log
 */
void listprocesses(struct processes *p){

    int i;
    char *d = DELIMITER;
    printf("\nrunning processes:\n");
    printf("-----------------\n");
    //printf("mricom pid: %d\n",p->mainpid);
    printf("PROC%sPARENT%sNAME%sPNAME%sTIMESTAMP\n",d,d,d,d);
    for(i=0; i<p->nproc;i++){
        printf("%d%s%d%s%s%s%s%s%s\n",
                p->pid[i],d,p->ppid[i],d,p->name[i],
                d,p->pname[i],d,p->timestamp[i]);
    }

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
//TODO
void killp(int procid){
    printf("killing process: %d ... ",procid);
    /* search through local process tree for check*/
    kill(procid, SIGTERM);
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
    return;
}

/*
 * Function: stimtest
 * ------------------
 * Run a brief digital trigger series
 *
 * Input: t -- stimulus length in seconds
 */

