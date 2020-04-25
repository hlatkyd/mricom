#include "common.h"
#define TESTLOG 1

/* This is an OK digital triggering solution without addititive timing error
 * and microsec resolution.
 *
 *
 */
int getusecdelay(struct timeval tv);
int testlog(FILE *fp, int n, int time);

