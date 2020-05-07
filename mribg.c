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
    char buffer[5];

    int fd_r; // pipe read end
    int fd_w; // pipe write end

    // check args:
    if(argc != 3){
        fprintf(stderr, "mribg: wrong number of inputs on start\n");
        exit(1);
    }

    signal(SIGINT, sighandler);
    gs = malloc(sizeof(struct gen_settings));
    mp = malloc(sizeof(struct mpid));
    parse_gen_settings(gs);
    fill_mpid(mp);
    processctrl_add(gs->mpid_file, mp, "START");

    fd_r = atoi(argv[1]);
    fd_w = atoi(argv[2]);
    read(fd_r, buffer, 5);
    printf("in buffer: %s\n",buffer);

    processctrl_add(gs->mpid_file, mp, "STOP");
    return 0;

}
