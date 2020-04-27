#include "common.h"

/* This is an OK digital triggering solution without addititive timing error
 * and microsec resolution.
 *
 *
 */
int getusecdelay(struct timeval tv);
double getsecdiff(struct timeval tv1, struct timeval tv2);
void append_log(FILE *fp, int n, int time, int usec_ttl1);
void prepare_log(FILE *fp, char *parent, struct blockstim_settings *bs);
void finish_log_header(FILE *fp);

