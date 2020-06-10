/*
 * test_run.c
 *
 * Simulates full automated run on a series of sequences. Mricom should be 
 * started as normal and mribg set to auto mode. 
 *
 * test_run plays the role of console host and the console as well.
 * On sequence start Vnmrj calls vnmrclient to set sequence and study name,
 * then another vnmrclient call with the argument 'start' signals to setup
 * stimulation and start the sequence. 
 *
 * the list of sequence names and associated stimulation is taken from the file
 * test_exp.txt. 
 *
 */

#include "common.h"

#define EXPFILE "test_exp.txt"

int read_exp(char expfile, struct study *st);

int main(it argc, char **argv){

    char exp_file[] = EXPFILE;
    struct study *st;

    read_exp(exp_file, st);
    st = malloc(sizeof(struct study));

    free(st);

    return 0;
}

int read_exp(char expfile, struct study *st){
   
    FILE *fp;
    fp = fopen(expfile,"r");
    if(fp == NULL){
        perror("fopen");
        exit(1);
    }

    return 0; 
}
