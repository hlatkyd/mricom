/* analogdaq.h
 *
 * Analog acquisition subprogram for Mricom. Intended for physiological data
 * coming from SA Instruments acquisition system
 *
 * base taken from comedilib demo, author: David A. Schleef <ds@schleef.org>
 */

#include "common.h"


struct ai_settings{

    


};


int set_smd_params(comedi_cmd *cmd, int p_ns, int n_scans);
void print_datum
