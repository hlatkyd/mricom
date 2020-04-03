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

/* daq constants */
#define NAICHAN 6 // number of analog input channels
#define NAOCHAN 0 // number of analog output channels
#define NDICHAN 3 // number of digital input channels
#define NDOCHAN 1 // number of digital output channels
#define NBUFFER 200 // sampled data buffer
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
    double window[NAICHAN][TIME_WINDOW * SAMPLING_RATE]; // data window in kst
    double membuf[NAICHAN][NBUFFER]; // full data buffer in memory
    double daqbuf[NAICHAN][NBUFFER]; // daq board data buffer
} daq_data;

typedef struct daq_settings{
    char device[32];
    char daq_file[128];         // full data file
    char ramdisk[128];          // ramdisk for fast data logging
    char procpar[128];          // vnmrj procpar file of curexp
    char event_dir[128];        // dir of stimulation event files
    //TODO is this needed?
    char sequence_file[128];    // file of mri sequence series and stimfiles
    char kst_settings[128];     // kst settings file
    char kst_path[128];         // path to kst2, found while init
    int channels;               // number of channels to save data from
    char channel_names[16][16]; // channel names in kst and data file
    char data_window_file[128]; // kst data file
    
}daq_settings;
extern daq_settings *settings;


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
extern daq_data *data;
// TODO render obsolete
extern acquisition_const *acqconst;
extern double **data_window; // this is what kst displays in realtime
extern double **data_buffer; // this is where new samples go
#endif
//TODO define user functions here?
