/*
 * mribg.c
 *
 * Background control program for mricom automation
 *
 */

#include "mribg.h"

int main(int argc, char **argv){

    // pid setup
    struct mpid *mp;
    struct gen_settings *gs;

    // fifo setup
    char mricomdir[LPATH];
    char bginfifo[LPATH] = {0};
    char bgoutfifo[LPATH] = {0};
    char init_msg[] = "mribg init successful...";
    fd_set set; 
    struct timeval timeout;
    int fd, ret;
    int rv;

    //socket setup
    

    if(getenv("MRICOMDIR") == NULL){
        fprintf(stderr, "mribg: environment varieble MRICOMDIR not set\n");
        exit(1);
    }

    signal(SIGTERM, sighandler);
    strcpy(mricomdir, getenv("MRICOMDIR"));

    // init
    gs = malloc(sizeof(struct gen_settings));
    mp = malloc(sizeof(struct mpid));
    memset(gs, 0, sizeof(struct gen_settings));
    memset(mp, 0, sizeof(struct mpid));
    parse_gen_settings(gs);
    fill_mpid(mp);
    processctrl_add(gs->mpid_file, mp, "START");

    // init fifo

    snprintf(bginfifo, sizeof(bginfifo), "%s/%s",mricomdir, BGINFIFO);
    snprintf(bgoutfifo, sizeof(bgoutfifo), "%s/%s",mricomdir, BGOUTFIFO);
    mkfifo(bginfifo, 0666);
    mkfifo(bgoutfifo, 0666);
    fd = open(bgoutfifo, O_WRONLY);
    write(fd, init_msg, strlen(init_msg));
    close(fd);

    while(1){

        sleep(5);
        //printf("mribg reporting in...\n");
    }

    // shutdown
    processctrl_add(gs->mpid_file, mp, "STOP");
    return 0;

}
