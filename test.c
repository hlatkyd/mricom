/* test.c
 *
 *
 *
 *
 */
#include "test.h"
#include "mricom.h"

void test_print(char **args){

    printf("test_print says hello\n");
    return;
}
void test_fork(){
    
    int i;
    pid_t pid;
    pid = fork();
    printf("hello forks!\n");
    
    if(pid == 0){
        for(i=0; i<100; i++){
            printf("sleep %d\n",i);
            sleep(10);
        }
    }
    else if(pid < 0){
        //error forking
        perror("error forking");
    }
    else {
        addpid(pid,&processes);
        printf("child pid %d\n",pid);
    }
    return;    

}
