/* analogdaq.h
 *
 * Analog acquisition subprogram for Mricom. Intended for physiological data
 * coming from SA Instruments acquisition system
 *
 * base taken from comedilib demo, author: David A. Schleef <ds@schleef.org>
 */

#include "common.h"


struct ai_settings{

    int subdev;
    int naichan;
    int aref;    
    unsigned int chanlist[NAICHAN]; // comedi spec, not the actual chan number
    comedi_range *range_info[NAICHAN];
    lsampl_t maxdata[NAICHAN];

    int n_scan;
    int period_ns;

};
int prepare_cmd_lib(comedi_t *dev, comedi_cmd *cmd, struct ai_settings *as);
int prepare_cmd(comedi_t *dev, comedi_cmd *cmd, struct ai_settings *as);

void print_ai_settings(struct ai_settings *as);
double to_physical(unsigned int sample, struct ai_settings *as, int ch);
//util grabbed from comedilib demos
void dump_cmd(FILE *out, comedi_cmd *cmd);
char *cmd_src(int src, char *buf);
