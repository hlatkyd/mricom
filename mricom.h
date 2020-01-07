/* mricom.h
 *
 * 
 *
 */

#define VERSION_MINOR 1
#define VERSION_MAJOR 0

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/* daq constants */
#define DAQ_FILE "/ramdisk/mricom.dat"
#define NACHAN 5 // number of analog input channels
#define NDCHAN 3 // number of digital input channels
#define NDATA 1024 // sampled data buffer
#define SAMPLING_RATE 200 // daq sampling rate in samples/s
#define TIME_WINDOW 20 // interval of time on charts in sec

/* constants for command history */
#define MAX_ID 16 // maximum number of processes
#define MAX_NAME_LENGTH 32 // maximum process name length
#define MAX_HISTORY_LENGTH 64 // command history elements
#define MAX_CMD_LENGTH 128 // command length to story in history

/* define global structs */

#ifndef MRICOM_H // header guard
#define MRICOM_H
typedef struct processes{
    int nproc;
    int procid[MAX_ID];
    char name[MAX_ID][MAX_NAME_LENGTH];
} processes;

typedef struct history{

    int n;
    char cmd[MAX_HISTORY_LENGTH][MAX_CMD_LENGTH];
}history;

typedef struct acquisition_data{
    int nchan;
    char name[NACHAN][32];
    double data[NACHAN][TIME_WINDOW * SAMPLING_RATE];
    
} acquisition_data;

extern processes *procpt;
extern history *cmdhist;
extern acquisition_data *acqdata;
#endif
//TODO define user functions here?
