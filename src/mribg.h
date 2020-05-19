/*
 * mribg.h
 *
 */

#include "common.h"

#define EXPFILE "experiment"

int parse_vnmrclient_msg(char *msg, int *argc, char **args);
int process_request(char *msg);
