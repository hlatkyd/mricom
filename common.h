/* mricom.h
 * --------
 * Device used: NiDAQ PCI-6035E
 *
 */

#define VERSION_MINOR 1
#define VERSION_MAJOR 0

// define debug
#define DEBUG 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <comedilib.h>

/* daq constants */
#define NAICHAN 3 // number of analog input channels
#define NAOCHAN 0 // number of analog output channels
#define NDICHAN 1 // number of digital input channels
#define NDOCHAN 1 // number of digital output channels
#define NCHAN  (NAICHAN+NAOCHAN+NDICHAN+NDOCHAN + 1) // +time
//NOTE: fifo is 512 samples on NI-6035E
#define NBUFFER 200 // sampled data buffer
#define BUFSZ 10000 // TODO test this, buffer size , comedi
#define SAMPLING_RATE 200 // daq sampling rate in samples/s
#define TIME_WINDOW 20 // interval of time on charts in sec
#define DELIMITER "\t"  // used in data file

/* settings file containing user defined stuff */
#define SETTINGS_FILE "settings"

#ifndef COMMON_H
#define COMMON_H

typedef struct daq_settings{
    char device[32];
    char daq_file[128];         // full data file
    FILE *fp_daq;               // FILE pointer to daq file if open
    char kst_file[128];         // only contain data for kst display window
    FILE *fp_kst;               // FILE pointer to kst window file if open
    int precision;              // decimals saved in data files
    char ramdisk[128];          // ramdisk for fast data logging
    char pid_file[128];          // local process id and contorl file
    FILE *fp_pid;               // FILE pointer to pidfile
    char procpar[128];          // vnmrj procpar file of curexp
    char event_dir[128];        // dir of stimulation event files
    //TODO is this needed?
    char sequence_file[128];    // file of mri sequence series and stimfiles
    char kst_settings[128];     // kst settings file
    char kst_path[128];         // path to kst2, found while init
    int channels;               // number of channels to save data from
    char channel_names[16][16]; // channel names in kst and data file
    // 0 or 1 to signal if acquisition is ongoing and prohibit some functions
    // for example test data generation, etc...
    // TODO remove these, make settings struct stable...
    int is_daq_on;
    int is_kst_on;              // 1 if a kst instance was started

}daq_settings;

// comedi device setup struct, most values can be set in settings
// depends on actual wiring setup, careful when settings these
typedef struct dev_settings{

    // comedi device pointer
    comedi_t *dev;
    // analog acquisition command
    comedi_cmd *cmd;
    // 1 if analog wiring is differential, 0 otherwise
    int is_analog_differential;
    // analog subdevice number (0 on ni-6035e)
    unsigned int analog_in_subdev;
    // analog channels, usually 0,1,2,....
    unsigned int analog_in_chan[8];
    // subdevice where digital stimulation channel is located (2)
    unsigned int stim_trig_subdev;
    // digital stim channel (0 in settings)
    unsigned int stim_trig_chan;
    //TODO what is this for again??
    double timing_buffer[1024];

} dev_settings;

struct blockstim_settings{

    char device[16];
    int subdev;                         // digital io subdevice on 6035E: 
    int chan;                           // digital output channel
    double start_delay;
    double on_time;
    double off_time;
    int ttl_usecw;
    double ttl_freq;
    int n_blocks;

};

extern daq_settings *settings;
extern dev_settings *devsettings;


#endif

int parse_procpar();
int search_procpar(char *parname, char *command);
int parse_settings(struct daq_settings *, struct dev_settings *);
int parse_blockstim_conf(struct blockstim_settings *bs, char *file, char *d);

/* util common func*/
void remove_spaces(char *);
int getppname(char *name);
