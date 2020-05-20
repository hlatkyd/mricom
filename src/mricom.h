/* mricom.h
 * --------
 * Device used: NiDAQ PCI-6035E
 *
 */

/* constants for command history */
#define MAX_HISTORY_LENGTH 64 // command history elements
#define MAX_CMD_LENGTH 128 // command length to story in history

/* ----------------------*/
/*     shell constants   */
/* ----------------------*/

#define CMD_BUFFER 256
#define MAX_ARG_LENGTH 128
#define ARG_DELIM " \t\r\n\a"

/* ----------------------*/
/* function declarations */
/* ----------------------*/

int mribg_launch();
/* ----------------------*/
/*  main shell functions */
/* ----------------------*/
void init();
void init_msg();
void exit_cleanup();
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
int sh_killp(int argc, char **args);
int sh_start(int argc, char **args);
int sh_stop(int argc, char **args);
int sh_list(int argc, char **args);
int sh_clean(int argc, char **args);
int sh_update(int argc, char **args);
int sh_send(int argc, char **args);

