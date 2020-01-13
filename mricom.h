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
#include <readline/readline.h>
#include <readline/history.h>

/* monitoring constants */
#define PROCPAR "/mnt/ramdisk/test.procpar"

/* daq constants */
#define DAQ_FILE "/mnt/ramdisk/mricomrt.dat"
#define NACHAN 5 // number of analog input channels
#define NDCHAN 3 // number of digital input channels
#define NDATA 1000 // sampled data buffer
#define SAMPLING_RATE 200 // daq sampling rate in samples/s
#define TIME_WINDOW 20 // interval of time on charts in sec
#define DELIMITER "\t"  // used in data file 

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

/* acquisition constants, channels, monitor files, etc*/
typedef struct acquisition_const{
    
    int chnum;
    int c_dwindow;
    int c_dbuffer;
    double sampling_rate;
    double time_window;
    char acqfile[128];
    char procpar_path[128];
    char chname[16][16];

} acquisition_const;

extern processes *procpt;
extern history *cmdhist;
extern acquisition_const *acqconst;
extern double **data_window; // this is what kst displays in realtime
extern double **data_buffer; // this is where new samples go
#endif
//TODO define user functions here?
