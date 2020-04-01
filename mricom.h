/* mricom.h
 * --------
 * Device used: NiDAQ PCI-6035E
 *
 */

#define VERSION_MINOR 1
#define VERSION_MAJOR 0

// define debug
#define DEBUG 1

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <comedilib.h>


//TODO move to settings file
/* USER SETTINGS */
#define RAMDISK "/mnt/ramdisk/"
#define PROCPAR "/mnt/ramdisk/test.procpar"
#define DAQ_FILE "/mnt/ramdisk/mricomrt.dat"
#define DEVICE "/dev/comedi0"


//TODO get this into settings file
/* daq constants */
#define NACHAN 6 // number of analog input channels, + time
#define NDCHAN 3 // number of digital input channels + time
#define NDATA 200 // sampled data buffer
#define SAMPLING_RATE 200 // daq sampling rate in samples/s
#define TIME_WINDOW 20 // interval of time on charts in sec
#define DELIMITER "\t"  // used in data file 

/* settings file containing user defined stuff */
#define SETTINGS_FILE "settings"

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

typedef struct daq_data{
    double window[NACHAN][TIME_WINDOW * SAMPLING_RATE]; // data window in kst
    double membuf[NACHAN][NDATA]; // full data buffer in memory
    double daqbuf[NACHAN][NDATA]; // daq board data buffer
} daq_data;

typedef struct daq_settings{
    char device[32];
    char daq_file[128];         // full data file
    char procpar_file[128];     // vnmrj procpar file of curexp
    char event_file[128];       // stimualtion event file
    char event_file_dir[128];   // dir of stimulation event files
    char sequence_file[128];    // file of mri sequence series and stimfiles
    char kst_file[128];         // kst settings file
    char data_window_file[128]; // kst data file
    int ndata;                  // number of data points in internal buffer
    int n_ai_chan;              // number of analog channels
    int n_di_chan;              // numver of digital input channels
    int n_do_chan;              // numver of digital output channels
    char achname[16][16];       // analog channel names
    char dchname[16][16];       // digital channel names
    int sampling_rate;          // samples per second
    int time_window;            // time window for kst graphs in seconds
    
}daq_settings;

//TODO render this obsolete
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
extern daq_settings *settings;
extern daq_data *data;
// TODO render obsolete
extern acquisition_const *acqconst;
extern double **data_window; // this is what kst displays in realtime
extern double **data_buffer; // this is where new samples go
#endif
//TODO define user functions here?
