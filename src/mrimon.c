/*
 * mrimon.c
 *
 * Read files such as log, sequence, and study files to display info.
 * To be used by the linux command watch outside of mricom. i.e: $ watch mrimon
 *
 *
 * To display:
 *
 * Study ID,
 * Elapsed time
 *
 * Sequence list, events, anesthesia
 *
 * Processses - only name, id, timestamp
 *
 *
 */

#include "common.h"
#include "func.h"
#include "socketcomm.h"

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

void printf_status(int status);

int main(int argc, char **argv){

    struct gen_settings *gs;
    struct processes *pr;
    struct study *st;
    char stsv_path[LPATH*2];
    struct timeval tv_start, tv_now;
    double elapsed_time;
    char response[64] = {0};
    int status;
    gettimeofday(&tv_now, NULL);
    st = malloc(sizeof(struct study));
    gs = malloc(sizeof(struct gen_settings));
    pr = malloc(sizeof(struct processes));
    memset(gs, 0, sizeof(struct gen_settings));
    memset(pr, 0, sizeof(struct processes));
    memset(st, 0, sizeof(struct study));
    parse_gen_settings(gs);
    processctrl_get(gs->mpid_file, pr);
    snprintf(stsv_path,sizeof(stsv_path),"%s/%sstudy.tsv",gs->workdir,DATA_DIR);

    extract_header_time(stsv_path, &tv_start);
    elapsed_time = getsecdiff(tv_start, tv_now);
    query_mribg("mrimon,get,status", response);
    status = atoi(response);

    printf(KWHT);
    printf("Elapsed time: %.3lf sec\n",elapsed_time);
    //printf("Status: %s\n", response);
    printf_status(status);
    printf(KNRM);
    listprocesses(pr);
    liststudy(gs);
    return 0;
}

void printf_status(int status){

    printf("Status: ");
    if(status == STATUS_MANUAL){
        printf("STATUS_MANUAL\n");
    }
    if(status == STATUS_AUTO_WAITING){
        printf("STATUS_AUTO_WAITING\n");
    }
    if(status == STATUS_AUTO_RUNNING){
        printf(KRED);
        printf("STATUS_AUTO_RUNNING\n");
        printf(KWHT);
    }
}
