/* mricom:  Collect data with NI card via comedi and control MRI
 * 
 * mricom.c: responsible for the interactive shell and general setup
 *
 * shell code base: Stephen Brennan
 * https://github.com/brenns10/lsh
 */

#include "mricom.h"
#include "func.h"
/* ----------------------*/
/*     shell constants   */
/* ----------------------*/

#define CMD_BUFFER 256
#define MAX_ARG_LENGTH 128
#define ARG_DELIM " \t\r\n\a"

/* ----------------------*/
/* acquisition constants */
/* ----------------------*/
const int channel_count = 6;
const char *channel_names[] = {"TIME","RESP", "PULSOX", "ECG", "TRIG", "GATE"};
const char procpar[] = PROCPAR;
const char daq_file[] = DAQ_FILE;

/* ----------------------*/
/* function declarations */
/* ----------------------*/

/*main shell functions*/
void init();
char **shell_parse_cmd(char *line);
char *shell_read_cmd();
void shell_loop();
int shell_launch(char **args);
int shell_execute(char **args);
/* ----------------------*/
/*        builtins       */
/* ----------------------*/

int sh_exit(char **args);
int sh_help(char **args);
int sh_test(char **args);
int sh_listh(char **args);
int sh_listp(char **args);
int sh_killp(char **args);
int sh_start(char **args);
int sh_stop(char **args);

/* init global process pointer */
processes *procpt;
/* init globla cmd history*/
history *cmdhist;
/* init global acquisition data */
acquisition_const *acqconst;
double **data_window;
double **data_buffer;


/* command names
 * 
 * should be the same order as builtin command pointer list */
char *builtin_str[] = {
    "exit",
    "help",
    "listh",
    "listp",
    "killp",
    "test",
    "start",
    "stop"
};
/* functions for builtin commands*/
/* should be same oreder as builtin_str list names*/
int (*builtin_func[]) (char **) = {
    &sh_exit,
    &sh_help,
    &sh_listh,
    &sh_listp,
    &sh_killp,
    &sh_test,
    &sh_start,
    &sh_stop
};
int sh_num_builtins(){
    return sizeof(builtin_str) / sizeof(char*);
}

/* builtin implementations */

/* Shell function: sh_exit
 * -----------------------
 * attempt to exit program gracefully
 */
int sh_exit(char **args){
    
    int i,j,n;
    int id;
    n = procpt->nproc;

    printf("Killing internal processes...\n");
    for(i=n;i>0;i--){
        id = procpt->procid[i-1];
        killp(id);
    }
    free(data_window);
    free(data_buffer);
    free(procpt);
    free(cmdhist);
    free(acqconst);
    return 0;
}
/* Shell function sh_help
 * ----------------------
 * prints available builtin commands
 */
//TODO help description to builtins
int sh_help(char **args){
    int i;
    printf("--- mricom v%d.%d ---\n\n",VERSION_MAJOR,VERSION_MINOR);
    printf("Available commands:\n");
    printf("===================\n");
    for(i = 0; i<sh_num_builtins(); i++){
        printf("  %s\n",builtin_str[i]);
    }
    return 1;
}
int sh_test(char **args){
    test_print(args);
    test_fork();
    //test_randfill_buffer(0.0);
    //test_system();
    return 1;
}
int sh_listh(char **args){
    listh();
    return 1;
}
int sh_listp(char **args){
    listp();
    return 1;
}
int sh_killp(char **args){
    char argin[10];
    int procid;
    printf("args : %s\n",args[1]);
    strcpy(argin, args[1]);
    procid = atoi(argin);
    killp(procid);
    return 1;
}
int sh_start(char **args){

    return 1;
}
int sh_stop(char **args){

    return 1;
}
/*-----------------------------------------------------------------------------*/
/*                               OPAQUE PARTS                                  */
/*-----------------------------------------------------------------------------*/
/* Function: init
 * ---------------
 * print name, version, initialize process struct, init command history
 */
void init(){

    int i;
    int r_dwindow, c_dwindow, l_dwindow;
    int r_dbuffer, c_dbuffer, l_dbuffer;
    /* rows: channels
     * cols: data points
     *
     *
     */

    printf("-----------------------------------------------\n");
    printf("mricom v%d.%d",VERSION_MAJOR,VERSION_MINOR);
    printf(" - MRI control software using comedi\n");
    printf("-----------------------------------------------\n");
    printf("Type 'help' to list available commands.\n");

    // malloc for process structure
    procpt = (processes*)malloc(sizeof(processes));
    procpt->nproc = 0;
    // malloc for command history
    cmdhist = (history*)malloc(sizeof(history));
    cmdhist->n = 0;

    // malloc for real time data and data buffer
    c_dwindow = SAMPLING_RATE * TIME_WINDOW;
    l_dwindow = SAMPLING_RATE * TIME_WINDOW * channel_count;
    r_dwindow = channel_count;
    c_dbuffer = NDATA;
    l_dbuffer = NDATA * channel_count;
    r_dbuffer = channel_count;

    // setup acquisition constants
    acqconst = (acquisition_const*)malloc(sizeof(acquisition_const));
    acqconst->chnum = channel_count;
    acqconst->c_dwindow = c_dwindow;
    acqconst->c_dbuffer = c_dbuffer;
    acqconst->time_window = (double)TIME_WINDOW;
    acqconst->sampling_rate = (double)SAMPLING_RATE;
    strcpy(acqconst->acqfile, daq_file);
    strcpy(acqconst->procpar_path, procpar);
    for(i = 0; i<channel_count; i++){
        strcpy(acqconst->chname[i], channel_names[i]);
    }

    //TODO malloc check
    data_window = (double **)malloc(sizeof(double*) * channel_count);
    for(i=0; i<r_dwindow; i++){
        data_window[i] = (double*)malloc(c_dwindow * sizeof(double));
    }
    data_buffer = (double **)malloc(sizeof(double*) * channel_count);
    for(i=0; i<r_dbuffer; i++){
        data_buffer[i] = (double*)malloc(c_dbuffer * sizeof(double));
    }
    if(data_window == NULL){
        printf("error: data_window malloc\n");
    }
    if(data_buffer == NULL){
        printf("error: data_buffer malloc\n");
    }

    // init daq file
    daq_init_kstfile();
}

/* Function: shell_parse_cmd
 * -------------------------
 * tokenizes the user input
 *
 * line: user input of the command and arguments separated by spaces
 *
 * returns: list of strings containing the command and arguments
 */

char **shell_parse_cmd(char *line){

    int bufsize = MAX_ARG_LENGTH;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if(!tokens){
        fprintf(stderr, "parse_cmd: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line,ARG_DELIM);
    while(token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += MAX_ARG_LENGTH;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens) {
                fprintf(stderr, "parse_cmd: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, ARG_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/* Function: shell_read_command
 * ----------------------------
 * reads user input into command buffer
 *
 * returns: string, carrage return replaced by '\0'
 */

char *shell_read_cmd(){

    int bufsize = CMD_BUFFER;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if(!buffer) {
        fprintf(stderr, "shell_read_line: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    printf(">>> ");
    while (1) {
    
        c = getchar();
        if(c == EOF || c == '\n'){
            buffer[position] = '\0';
            if (strlen(buffer) != 0){
                append_to_history(buffer);
            }
            return buffer;
        }
        else {
            buffer[position] = c;
        }
        position++;

        if(position >= bufsize) {
            bufsize += CMD_BUFFER;
            buffer = realloc(buffer,bufsize);
            if(!buffer) {
                fprintf(stderr, "shell_read_line: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}
/* Function: shell_launch
 * ----------------------
 * launches original shell commmands 
 */
int shell_launch(char **args){
    
    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0){
        // child process
        if(execvp(args[0], args) == -1){
            printf("args[0]: %s\n",args[0]);
            perror("shell_launch");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0){
        //error forking
        perror("shell_launch");
    }
    else {
        // parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}
/* Function: shell_execute
 * -----------------------
 * executes builtin command if name matches, othervise execute system command
 */
int shell_execute(char **args){
    int i;

    if(args[0] == NULL) {
        //empty command
        return 1;
    }

    for(i=0; i<sh_num_builtins();i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }
    return shell_launch(args);
}

void shell_loop(){

    char *line;
    char **args; 
    int res;

    do {
        line = shell_read_cmd();

        args = shell_parse_cmd(line);

        res = shell_execute(args);

        free(line);
        free(args);
    } while(res != 0);

}

int main(int argc, char *argv[]){

    init();

    shell_loop();

    return EXIT_SUCCESS;
}
