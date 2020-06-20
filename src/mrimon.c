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


int main(int argc, char **argv){

    struct gen_settings *gs;
    struct processes *pr;
    struct study *st;
    char curstd_path[LPATH*2];
    struct timeval tv_start, tv_now;

    gettimeofday(&tv_now, NULL);
    st = malloc(sizeof(struct study));
    gs = malloc(sizeof(struct gen_settings));
    pr = malloc(sizeof(struct processes));
    memset(gs, 0, sizeof(struct gen_settings));
    memset(pr, 0, sizeof(struct processes));
    memset(st, 0, sizeof(struct study));

    parse_gen_settings(gs);
    processctrl_get(gs->mpid_file, pr);
    extract_header_time(curstd_path, &tv_start);
    listprocesses(pr);
    liststudy(gs);
    return 0;
}
