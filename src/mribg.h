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
int process_request(struct gen_settings *gs, char *msg, char *msg_response);
int mribg_status_check(int state);
int fork_blockstim(char **args);
int fork_analogdaq(char **args);
int fork_ttlctrl(char **args);
int fork_mriarch(char **args);

// main data util
int datahandler(struct gen_settings, struct study *st, char *action);
// util
void create_study_dir(struct gen_settings *gs, struct study *stud);
void create_sequnce_dir(struct gen_settings *gs, struct study *stud);
