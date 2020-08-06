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
    printf("\nstim_trig_subdev: %d\n",devsettings->stim_subdev);
    printf("stim_trig_chan: %d\n",devsettings->stim_trig_chan);
    printf("stim_ttl_chan: %d\n",devsettings->stim_ttl_chan);
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
    //printf("daq_file : %s\n",settings->daq_file);
    printf("mpid_file : %s\n",settings->mpid_file);
    printf("kst_file : %s\n",settings->kst_file);
    printf("workdir : %s\n",settings->workdir);
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
}

void liststudy(struct gen_settings *gs){

    FILE *fp;
    char path[LPATH*2];
    size_t len = 0;
    ssize_t read;
    char *line = NULL;
    int count = -1;
    char *tok;
    char id[16];

    snprintf(path, sizeof(path), "%s/%sstudy.tsv",gs->workdir,DATA_DIR);
    fp = fopen(path, "r");
    if(fp == NULL){
        fprintf(stderr, "cannot  open file %s\n",path);
        return;
    }
    while((read = getline(&line, &len, fp)) != -1){
        // find study id line
        if(strncmp(line, "id=",3)==0 && count == -1){
            count = 0;
            tok = strtok(line, "=");
            tok = strtok(NULL, "=");
            printf("\nStudy ID: %s", tok);
            printf("---------------------------------------"
                   "---------------------------------------\n");
        // print rest
        } else if (count > 0){
            printf("%s",line);
            count++;
        // column lines
        } else if(strncmp(line, "seqnum",6)==0){
            printf("Num\tSequence\t\tEvent\t\tTime\n");
            printf("======================================="
                   "=======================================\n");
            count++;
        } else {
            continue;
        }
    }
    fclose(fp);

}

/*
 * Function: listprocesses
 * -----------------------
 * list running processes from mpid.log
 */
void listprocesses(struct processes *p){

    int i,j;
    char *d = DELIMITER;
    printf("\nrunning processes: %d, mricom: %d, mribg: %d\n",
            p->nproc, p->mainpid, p->bgpid);
    printf("---------------------------------------"
           "---------------------------------------\n");
    //printf("mricom pid: %d\n",p->mainpid);
    printf("PROC%s%s%s%-8s%s%-8s%sTIMESTAMP\n",
            d,"PARENT",d,"NAME",d,"PNAME",d);
    for(i=0; i<p->nproc;i++){
        printf("%d%s%d%s%-8s%s%-8s%s%s\n",
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

int stop_mribg(int pid){

    char procname[32] = {0};
    int ret;
    getname(procname, pid);
    if(strcmp(procname, "mribg")==0){
        ret = kill(pid, SIGINT);
        if(ret < 0){
            perror("stop_mribg");
            exit(1);
        }
        return 0;
    } else {
        fprintf(stderr, "stop_mribg: found non-matching process name %s\n",
                procname);
        return -1;
    }
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

