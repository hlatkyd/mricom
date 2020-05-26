/* blockstim.h
 *
 * This is an OK digital triggering solution without addititive timing error
 * and microsec resolution.
 *
 *
 */

#include "common.h"

/* ---------------------------------*/
/*       comedi subprogram settings */
/* ---------------------------------*/
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
    int trig_on;
    int trig_chan;

};
void append_bs_data(FILE *fp, int n, int b, int time, int usec_ttl1);
void append_bs_chdata(FILE *fp, struct blockstim_settings *bs);
int parse_bstim_conf(struct blockstim_settings *bs, char *file, char *d);
void fprintf_bstim_meta(char *p, struct header *h, struct blockstim_settings *bs);

// troubleshooting
void printf_bs(struct blockstim_settings *bs);
