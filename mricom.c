/* mricom:  Collect data with NI card via comedi and control MRI
 * 
 * mricom.c: responsible for the interactive shell
 *
 * shell code base: Stephen Brennan
 * https://github.com/brenns10/lsh
 */

#include "mricom.h"
#include "test.h"

/* ----------------------*/
/*     shell constants   */
/* ----------------------*/

#define CMD_BUFFER 256
#define MAX_ARG_LENGTH 128
#define ARG_DELIM " \t\r\n\a"

typedef struct process{
    int nproc;      // number of live builtin shell processes
    int procid[16]; // builtin shell process ids
    char *name[32]
} process;

/* ----------------------*/
/* function declarations */
/* ----------------------*/

void init();
char **shell_parse_cmd(char *line);
char *shell_read_cmd();
int shell_launch(char **args);
int shell_execute(char **args);

/* ----------------------*/
/*        builtins       */
/* ----------------------*/

int sh_exit(char **args);
int sh_help(char **args);
int sh_test(char **args);

/* command names */

char *builtin_str[] = {
    "exit",
    "help",
    "test",
    "startgen",
    "stopgen"
};
/* functions for builtin commands*/
int (*builtin_func[]) (char **) = {
    &sh_exit,
    &sh_help,
    &sh_test
};
int sh_num_builtins(){
    return sizeof(builtin_str) / sizeof(char*);
}

/* builtin implementations */

/* Shell function: sh_exit
 * -----------------------
 * exit program
 */
int sh_exit(char **args){
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
    test_print();
    return 1;
}
/*-----------------------------------------------------------------------------*/

/* Function: init
 * ---------------
 * print name, version
 */
void init(){

    printf("mricom v%d.%d",VERSION_MAJOR,VERSION_MINOR);
    printf(" - MRI control software using comedi\n");
    printf("Type 'help' to list available commands.\n");
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
    
    while (1) {
    
        c = getchar();
        if(c == EOF || c == '\n'){
            buffer[position] = '\0';
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
        printf(">>> ");
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
