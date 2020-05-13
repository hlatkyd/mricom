/*
 * mribg.c
 *
 * Background control program for mricom automation
 *
 */

#include "mribg.h"

int main(int argc, char **argv){

    struct mpid *mp;
    struct gen_settings *gs;

    int a = 1;
    char mribg_log[128];
    char buffer[8];
    char initmsg[8];

    int fd_r; // pipe read end
    int fd_w; // pipe write end

    // check args:
    if(argc != 3){
        fprintf(stderr, "mribg: wrong number of inputs on start\n");
        exit(1);
    }
    printf("mribg: started\n");
    signal(SIGINT, sighandler);
    gs = malloc(sizeof(struct gen_settings));
    mp = malloc(sizeof(struct mpid));
    memset(gs, 0, sizeof(struct gen_settings));
    memset(mp, 0, sizeof(struct mpid));
    parse_gen_settings(gs);
    fill_mpid(mp);
    snprintf(initmsg,sizeof(initmsg),"%d",mp->pid);
    processctrl_add(gs->mpid_file, mp, "START");

    fd_r = atoi(argv[1]);
    fd_w = atoi(argv[2]);
    //read(fd_r, buffer, 5);
    write(fd_w, initmsg, sizeof(initmsg));
    //printf("in buffer: %s\n",buffer);

    while(1){


        sleep(5);
        //printf("mribg report\n");
    }
    processctrl_add(gs->mpid_file, mp, "STOP");
    return 0;

}
