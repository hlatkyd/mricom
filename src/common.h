/* common.h
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
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/mman.h>
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
#define SETTINGS_FILE "conf/settings"
#define MPROC_FILE "mproc.log"
#define BIN_DIR "bin/"
#define CONF_DIR "conf/"
#define DATA_DIR "data/"
// max path length
#define LPATH 128

#ifndef COMMON_H
#define COMMON_H

/* -------------------------------*/
/*   mribg status handler         */
/* -------------------------------*/

int mribg_status;

/* -------------------------------*/
/*  process pid file control      */
/* -------------------------------*/

struct mpid{

    int num; // TODO let mricom issue this<
    struct timeval tv;
    char name[32];
    char pname[32];
    pid_t pid;
    pid_t ppid;

};

/* -------------------------------*/
/*    general daq settings        */
/* -------------------------------*/
typedef struct gen_settings{
    char device[32];
    char workdir[LPATH];          // acquisition directory
    char mpid_file[LPATH];          // local process id and contorl file

    char bginfifo[LPATH];
    char bgoutfifo[LPATH];

    char daq_file[LPATH];         // full data file
    FILE *fp_daq;               // FILE pointer to daq file if open
    char kst_file[128];         // only contain data for kst display window
    FILE *fp_kst;               // FILE pointer to kst window file if open
    int precision;              // decimals saved in data files
    char ramdisk[128];          // ramdisk for fast data logging
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
/* -------------------------------*/
/*    general device settings     */
/* -------------------------------*/
typedef struct dev_settings{

    char devpath[16];   // device file
    comedi_t *dev;      // comedi device pointer
    comedi_cmd *cmd;    // analog acquisition command
    int is_analog_differential; // 1 if analog wiring is differential
    int analog_sampling_rate;
    char analog_ch_names[16][16]; // names of analog chs for data files, etc
    int analog_in_subdev; // analog subdevice number (0 on ni-6035e)
    int analog_in_chan[8];// analog channels, usually 0,1,2,....
    int stim_trig_subdev;//subdev of  digital stim channel
    int stim_trig_chan;// digital stim channel (0 in settings)
    int ttlctrl_subdev;
    int ttlctrl_in_chan;
    int ttlctrl_out_chan;
    int ttlctrl_usr_chan[3];


} dev_settings;

/* -------------------------------*/
/*            headers             */
/* -------------------------------*/

struct header{

    double version;
    char proc[16];
    char args[16][32];
    struct timeval timestamp;
    struct timespec timestamp2;

    char names[32][32];
    char units[32][32];
};

struct times{

    //TODO replace these with clock_gettime
    struct timeval start;
    struct timeval action;
    struct timeval stop;

    // TODO use with clock_gettime()
    struct timespec cstart;
    struct timespec caction;
    struct timespec cstop;
};

/* -------------------------------*/
/*      running processes         */
/* -------------------------------*/

#define MAX_ID 64 // maximum number of processes
#define MAX_NAME_LENGTH 32 // maximum process name length
/* procerss struct to keep track of local child processes*/
struct processes{

    int mainpid;
    int bgpid;
    int nproc;
    int pid[MAX_ID];
    int ppid[MAX_ID];
    char name[MAX_ID][MAX_NAME_LENGTH];
    char pname[MAX_ID][MAX_NAME_LENGTH];
    char timestamp[MAX_ID][MAX_NAME_LENGTH];
};

#endif
/*------------------------------*/
/*      common function declare */
/*------------------------------*/

int parse_procpar();//TODO why?
int search_procpar(char *parname, char *command); //TODO check again
int parse_dev_settings(struct dev_settings *);
int parse_gen_settings(struct gen_settings *);
void fprintf_times_meta(FILE *fp, struct times *t); // TODO remove,add types

/* process control */

void fill_mpid(struct mpid *mp);
int processctrl_add(char *path, struct mpid *mp, char *status);
int processctrl_get(char *path, struct processes *p);
int processctrl_clean(struct gen_settings *gs, struct processes *pr);
int processctrl_archive(char *path, char *archive_path);
void sighandler(int signum);

/* util common func*/
void remove_spaces(char *);
void getppname(char *name);
void getname(char *name, int pid);
void getcmdline(char *cmd);
void gethrtime(char *buffer, struct timeval tv);
void getclockhrtime(char *buffer, struct timespec tv);
int getusecdelay(struct timeval tv1);
int clockusecdelay(struct timespec tv1);
double getsecdiff(struct timeval tv1, struct timeval tv2);
double clocksecdiff(struct timespec tv1, struct timespec tv2);

/* data file management */

void fprintf_meta_times(char *p, struct times *t, char *element);
void fprintf_meta_intrpt(char *p);
int fprintf_common_header(FILE *fp, struct header *h, int argc, char **args);
int compare_common_header(char *file1, char *file2);


/* archiving and finishing up experiment session*/
//TODO everything here
int finalize_session(); // maain function at the end of experiment
// can run these after a seqeunce has finished
int sort_analogdaq_data(); // separate analog input data for each sequence
int sort_blockstim_data(); 
int sort_eventstim_data(); // TODO eventstim sometime
//TODO maybe run xrecon here and not from vnmrj?

//
int bash_call();

