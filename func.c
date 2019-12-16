/* func.c
 *
 * mricom user function definitions
 */

#include "mricom.h"
#include "func.h"

extern processes *procpt;
extern history *cmdhist;

/*-------------------------------------------------------------------*/
/*                           test functions                          */
/*-------------------------------------------------------------------*/
void test_print(char **args){
    printf("test_print says hello\n");
    return;
}
void test_fork(){

    int i;
    int procnum;
    char testname[] = "test_process";
    char kstpath[] = "/usr/bin/kst2";
    char *args[] = {"/home/david/dev/mricom/test/kst_test.kst", NULL};
    pid_t pid;
    pid = fork();
    printf("hello forks!\n");

    if(pid == 0){

        // child
        //system("kst2");
        execvp(kstpath, args);
        //perror("execv");
        return;
    }
    else if(pid < 0){
        //error forking
        perror("error forking");
    }
    else {
        // parent
        addprocess(pid, testname);
    }
    return;

}
void test_generate_data(){

    int nchan = 4;
    int npoints = 2000;
    int srate = 50;
    double **data;

    char testfile[] = "/home/david/dev/mricom/test/data.txt";
    FILE *fp;

    data = (double)malloc(srate * nchan * sizeof(double));



}
void test_write_data()P

    char testfile[] = "/home/david/dev/mricom/test/data.txt";
    FILE *fp;
    return;
}
void test_system(){
    /*
    char kstpath[] = "/usr/bin/kst2";
    char *args[] = {"/home/david/dev/mricom/kst_test.kst", NULL};
    execvp(kstpath, args);
    perror("execv");
    */
    return;
}
/*-------------------------------------------------------------------*/
/*                     util shell functions                          */
/*-------------------------------------------------------------------*/
void addpid(int pid){
    printf("adding pid: %d\n");
    return;
}
void addprocess(int pid, char *procname){
    
    int n;
    printf("adding process '%s' with pid %d\n",procname, pid); 
    n = procpt->nproc;
    procpt->procid[n] = pid;
    strcpy(procpt->name[n], procname);
    procpt->nproc = n + 1;
}
void append_to_history(char *buffer){

    int num;
    num = cmdhist->n;
    strcpy(cmdhist->cmd[num], buffer);
    cmdhist->n = num + 1;
    return;
}

/*-------------------------------------------------------------------*/
/*                     util user functions                           */
/*-------------------------------------------------------------------*/
void getproc(){
    return;
}
void listh(){
    
    int i;
    printf("Command history: \n");
    for(i=0;i<cmdhist->n; i++){
        printf("%s\n",cmdhist->cmd[i]);
    }
    return;
}
void listp(){

    int i;
    int n = procpt->nproc;
    printf("Mricom processes running:\n");
    if(n!=0){
        for (i = 0; i < n; i++){
            printf("name: %s, pid: %d\n",
                            procpt->name[i], procpt->procid[i]);
        }
    }
    return;
}

void killp(int procid){
    printf("killing process: %d ...",procid);
    kill(procid, SIGTERM);
    printf("done\n");
    return;
}
