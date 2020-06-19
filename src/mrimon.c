/*
 * mrimon.c
 *
 * Read files such as log, sequence, and study files to display info.
 * To be used by the linux command watch outside of mricom. i.e: $ watch mrimon
 */

#include "common.h"
#include "func.h"

int main(int argc, char **argv){

    struct gen_settings *gs;
    struct proceses, *p;

    gs = malloc(sizeof(struct gen_settings));
    p = malloc(sizeof(struct processes));

    processctrl_get(gs->mpid_file, pr);
    listprocesses(pr);
    return 0;
}
