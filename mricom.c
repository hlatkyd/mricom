/* mricom:  Collect data with NI card via comedi and control MRI
 * 
 * mricom.c: responsible for the interactive shell and general setup
 *
 * shell code base: Stephen Brennan
 * https://github.com/brenns10/lsh
 *
 * contents:
 * - 
 *
 *
 *
 *
 */

#include "mricom.h"

/* ----------------------*/
/*     shell constants   */
/* ----------------------*/

#define CMD_BUFFER 256
#define MAX_ARG_LENGTH 128
#define ARG_DELIM " \t\r\n\a"

/* ----------------------*/
/* function declarations */
/* ----------------------*/

/*main shell functions*/
void init();
char **shell_parse_cmd(char *line);
int shell_get_argc(char **args);
char *shell_read_cmd();
void shell_loop();
int shell_launch(int argc, char **args);
int shell_execute(int argc, char **args);
/* ----------------------*/
/*        builtins       */
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
int sh_stimtest(int argc, char **args);

/* init settings */
daq_settings *settings;
/* init data struct */
daq_data *data;
/* init global process pointer */
processes *procpt;
/* init device settings struct*/
dev_settings *devsettings;


/* command names
 * 
 * should be the same order as builtin command pointer list */
char *builtin_str[] = {
    "exit",
    "help",
    "listp",
    "killp",
    "test",
    "start",
    "stop",
    "list",
    "stimtest"
};
/* functions for builtin commands*/
/* should be same oreder as builtin_str list names*/
int (*builtin_func[]) (int, char **) = {
    &sh_exit,
    &sh_help,
    &sh_listp,
    &sh_killp,
    &sh_test,
    &sh_start,
    &sh_stop,
    &sh_list,
    &sh_stimtest
};
int sh_num_builtins(){
    return sizeof(builtin_str) / sizeof(char*);
}

/* builtin implementations */

/* Shell function: sh_exit
 * -----------------------
 * attempt to exit program gracefully
 */
 //TODO check for unintended stop ps -ef, or something
int sh_exit(int argc, char **args){
    
    int i,j,n;
    int id;
    n = procpt->nproc;

    printf("Killing internal processes...\n");
    for(i=n;i>0;i--){
        id = procpt->procid[i-1];
        killp(id);
    }
    comedi_device_close();
    if(settings->fp_daq != NULL)
        fclose(settings->fp_daq);
    if(settings->fp_kst != NULL)
        fclose(settings->fp_kst);
    free(procpt);
    free(data);
    free(settings);
    free(devsettings);
    return 0;
}
/* Shell function sh_help
 * ----------------------
 * prints available builtin commands
 */

int sh_help(int argc, char **args){
    int i, n;
    char *cmdname;
    cmdname = NULL;
    if(args[1] == NULL){
        printf("--- mricom v%d.%d ---\n\n",VERSION_MAJOR,VERSION_MINOR);
        printf("Available commands:\n");
        printf("===================\n");
        for(i = 0; i<sh_num_builtins(); i++){
            printf("  %s\n",builtin_str[i]);
        }
    }
    else{
        for(i=0; i<sh_num_builtins(); i++){
            if(strcmp(builtin_str[i],args[1])==0){
                cmdname = args[1];
                n = i;
            }
        }
        if(cmdname == NULL){
            printf("No such command as '%s'\n",args[1]);
            return 1;
        }
        printf("COMMAND: %s\n", cmdname);
        printf("---------------\n");
        switch (n)
        {
            case 0: // exit
                printf("exit mricom gracefully\n");
                break;
            case 1: // help
                printf("print available commands or get help for a command\n");
                break;
            case 2: // listp
                printf("list mricom child processes\n");
                break;
            case 3: // killp
                printf("kill mricom childprocess by id\n");
                break;
            case 4: // test
                printf("test everything?\n");
                break;
            case 5: // start
                printf("start data acquisition\n");
                break;
            case 6: // stop
                printf("stop data acquisition\n");
                break;
            case 7: // list
                printf("print settings struct\n");
                break;
            case 8: // stimtest
                printf("test digital trigger output\n");
                break;
        }
        printf("\n");

    }
    return 1;
}
int sh_test(int argc, char **args){
    //TODO implement usage by specific arguments
    // start, stop, stim etc
    clock_t time;
    double tot_time;
    int i;
    int n = 10;
    if(settings->is_daq_on == 1){
        printf("acquisition is ON, testing prohibited\n");
        return 1;
    }
    if(argc > 1){
        if(strcmp(args[1],"sim") == 0){
            printf("generating random data... \n");
            test_daq_simulate();
        } else if(strcmp(args[1],"trig") == 0){
                comedi_digital_trig("events/testevent.evt");
        } else if(strcmp(args[1],"analog") == 0){
            printf("testing analog setup\n");
            comedi_setup_analog_acq();
        } else if(strcmp(args[1],"digital") == 0){
            printf("testing digital setup and exec\n");
            comedi_setup_digital_sequence("test");
            comedi_execute_digital_sequence();
        } else if(strcmp(args[1],"timing") == 0){
            printf("testing execution time of some functions: \n");
            // filling bffer with random num
            time = clock();
            for(i=0;i<n;i++){
                test_rand_membuf(0);
            }
            time = clock() - time;
            tot_time = ((double)time)/CLOCKS_PER_SEC;
            printf(" test_rand_membuf: %f\n",tot_time / n);

        } else {
            test_print(args);
        }
    } else {
        printf(" (no action)\n");
    }
    //test_fork();
    //test_system();
    return 1;
}
int sh_listp(int argc, char **args){
    listp();
    return 1;
}
int sh_killp(int argc, char **args){
    char argin[10];
    int procid;
    //printf("args : %s\n",args[1]);
    strcpy(argin, args[1]);
    procid = atoi(argin);
    killp(procid);
    return 1;
}
int sh_start(int argc, char **args){
    //TODO make arguments useful, eg kst start

    if(argc == 2){
        if(strcmp(args[1],"testproc") == 0){
            launch_process("testproc");
        }
    }
    if(argc == 1)
        start();
    return 1;
}
int sh_stop(int argc, char **args){
    // TODO make arguments as well, eg kst stop
    stop();
    return 1;
}
int sh_list(int argc, char **args){
    //TODO maybe arguments here as well such as list [args, eg set]??
    if(argc > 1){
        if(strcmp(args[1],"settings")==0){
            double elapsed_time;
            printf("\n");
            listsettings();
            listdevsettings();
            printf("\n");
            elapsed_time = daq_timer_elapsed();
            printf("elapsed time = %lf\n",elapsed_time);
            return 1;
        } else if (strcmp(args[1],"process")==0){
            listp();
            return 1;
        } else {
            printf("unknown argument %s\n",args[1]);
            return 1;
        }
    }
}
int sh_stimtest(int argc, char **args){

    int t_length; // stimulus length in seconds
    int t_length_def = 10;
    int val;
    if(argc == 1){
        t_length = t_length_def;
    } else if(argc == 2){
        val = atoi(args[1]);
        if(val == 0){
            printf("stimtest: wrong input for stimulus length\n");
            t_length = t_length_def;
        } else {
            t_length = val;
        }
    }
    stimtest(t_length);
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

    int is_dev_ok, ret;

    printf("\n-----------------------------------------------\n");
    printf("mricom v%d.%d",VERSION_MAJOR,VERSION_MINOR);
    printf(" - MRI control software using comedi\n");
    printf("-----------------------------------------------\n");

    // malloc for settings
    settings = (daq_settings*)malloc(sizeof(daq_settings));
    // malloc for device settings
    devsettings = (dev_settings*)malloc(sizeof(dev_settings));
    parse_settings();
    settings->is_daq_on = 0;
    settings->is_kst_on = 0;

    /* check for kst2 install and ramdisk mount and device */

    if(DEBUG > 0){
        printf("\nmricom startup check...\n");
    }
    if(is_ramdisk_accessible() != 0){
        printf("\n\tWarning: ramdisk mount point not found!\n");
    }
    if(is_kst_accessible() != 0){
        printf("\n\tWarning: installed kst2 not found!\n");
    }
    if(is_nicard_accessible() != 0){
        printf("\n\tWarning: NI card not found!\n");
        is_dev_ok = 0;
    } else {
        is_dev_ok = 1;
    }

    //is_accessible(kstpath);

    // malloc for process structure
    procpt = (processes*)malloc(sizeof(processes));
    procpt->nproc = 0;
    procpt->mainpid = getpid();

    // malloc for daq_data
    data = (daq_data*)malloc(sizeof(daq_data));

    launch_process("procmonitor");

    // init daq file
    daq_init_kstfile();
    daq_timer_start();
    // init device
    if(is_dev_ok == 1)
        ret = comedi_device_setup();
        if(ret != 0)
            printf("\twarning: unable to set up comedi device\n");

    printf("\nType 'help' to list available commands.\n");
    printf("-----------------------------------------------\n");
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
 * reads user input into command buffer using readline lib
 *
 * returns: command string
 */

//TODO tab completion in readline
char *shell_read_cmd(){

    int i;
    char *buffer; // no malloc needed if using readline lib
    while(1){
        if((buffer = readline(">>> ")) != NULL){
            if(strlen(buffer) > 0){
                add_history(buffer);
                return buffer;
            }
        }
        else{
            fprintf(stderr, "shell_read_line: allocation error\n");
            exit(EXIT_FAILURE);
        }
    }

}

/* Function: shell_get_argc
 * ----------------------
 * counts argument strings 
 */
int shell_get_argc(char **args){

    int i = 0;
    while(1){
        if(args[i] != NULL){
            i++;
            continue;
        } else
            break;
    }
    return i;
}
/* Function: shell_launch
 * ----------------------
 * launches original shell commmands 
 */
int shell_launch(int argc, char **args){
    
    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0){
        // child process
        if(execvp(args[0], args) == -1){
            printf("%s: No such command\n",args[0]);
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
int shell_execute(int argc, char **args){
    int i;

    if(args[0] == NULL) {
        //empty command
        return 1;
    }
    /* prohibited shell commands*/
    if(strcmp(args[0],"kill") == 0){
        printf("did you mean 'killp [id]?'\n");
        return 1;
    }
    if(strcmp(args[0],"rm") == 0){
        printf("no 'rm' here!\n");
        return 1;
    }
    /* starting builtins if found*/
    for(i=0; i<sh_num_builtins();i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(argc, args);
        }
    }
    return shell_launch(argc, args);
}

void shell_loop(){

    char *line;
    char **args; 
    int res;
    int argc;

    do {
        line = shell_read_cmd();

        args = shell_parse_cmd(line);

        argc = shell_get_argc(args);

        res = shell_execute(argc, args);

        free(line);
        free(args);
    } while(res != 0);

}

int main(int argc, char *argv[]){

    init();

    shell_loop();

    return EXIT_SUCCESS;
}
