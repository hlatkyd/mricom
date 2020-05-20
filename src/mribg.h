/*
 * mribg.h
 *
 */

#include "common.h"
#include "socketcomm.h"

#define EXPFILE "experiment"

int parse_vnmrclient_msg(char *msg, int *argc, char **args);
int process_request(char *msg);
int fork_blockstim(char **args);
