/* mricom.h
 * --------
 * Device used: NiDAQ PCI-6035E
 *
 */

/* constants for command history */
#define MAX_ID 16 // maximum number of processes
#define MAX_NAME_LENGTH 32 // maximum process name length
#define MAX_HISTORY_LENGTH 64 // command history elements
#define MAX_CMD_LENGTH 128 // command length to story in history


/* ----------------------*/
/*     shell constants   */
/* ----------------------*/

#define CMD_BUFFER 256
#define MAX_ARG_LENGTH 128
#define ARG_DELIM " \t\r\n\a"


/* define global structs */

#ifndef MRICOM_H // header guard
#define MRICOM_H

/* procerss struct to keep track of local child processes*/
struct processes{
    int mainpid;
    int nproc;
    int procid[MAX_ID];
    char name[MAX_ID][MAX_NAME_LENGTH];
};


#endif

/* ----------------------*/
/* function declarations */
/* ----------------------*/

/* ----------------------*/
/*  main shell functions */
/* ----------------------*/
void init();
char **shell_parse_cmd(char *line);
int shell_get_argc(char **args);
char *shell_read_cmd();
void shell_loop();
int shell_launch(int argc, char **args);
int shell_execute(int argc, char **args);
/* ----------------------*/
/*     shell builtins    */
/* ----------------------*/

int sh_exit(int argc, char **args);
int sh_help(int argc, char **args);
int sh_test(int argc, char **args);
int sh_listh(int argc, char **args);
int sh_listp(int argc, char **args);
int sh_killp(int argc, char **args);
int sh_start(int argc, char **args);
int sh_stop(int argc, char **args);
int sh_list(int argc, char **args);

