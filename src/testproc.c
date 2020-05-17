/* Only for testing process control */

#include "common.h"

int main(int argc, char **argv){

    //TODO catch ctrl c and set log stop before exit
    signal(SIGINT, sighandler);
    struct mpid *mp;
    // get path from settings??
    char path[] = "mproc.log";
    mp = malloc(sizeof(struct mpid));
    fill_mpid(mp);
    processctrl_add(path, mp, "START");
    printf("Testproc running...\n");
    //printf("parent: %s\n",mp->pname);
    //printf("timeval %ld\n",mp->tv.tv_sec);
    //printf("name: %s\n",mp->name);
    sleep(30);
    printf("Testproc finished...\n");
    //processctrl_remove(path, mp);
    processctrl_add(path, mp, "STOP");
    return 0;
}
