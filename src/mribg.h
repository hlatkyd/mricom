/*
 * mribg.h
 *
 */

#include "common.h"
#include "socketcomm.h"

#define EXPFILE "experiment"

/* mrbibg status handlers */

#define STATUS_MANUAL 0
#define STATUS_AUTO_WAITING 1
#define STATUS_AUTO_RUNNING 2

int parse_vnmrclient_msg(char *msg, int *argc, char **args);
int process_request(char *msg, char *msg_response);
int fork_blockstim(char **args);
int fork_analogdaq(char **args);
int fork_ttlctrl(char **args);
int mribg_status_check();
