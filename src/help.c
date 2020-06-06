/*
 * help.c
 *
 * Print help for mricom commands
 */

#include "help.h"


void printf_help_exit(){

    printf("Shut down mricom and mribg gracefully\n");
}
void printf_help_help(){

    printf("Print available commands or get help for a command if given "
           "as an argument. Usage: help [arg]\n");
}
void printf_help_killp(){
    printf("Force kill a process. Usage: killp [process ID]\n");
}

void printf_help_test(){
}
void printf_help_start(){
    printf("Start stuff: automated aqciusition, kst, etc. "
            "Usage: start [auto,kst]\n");
}
//TODO
void printf_help_stop(){
    printf("Stop data acquisition or mribg??\n");
}
void printf_help_list(){
    printf("List some stuff: settings, parameters, processes. "
            "Usage: list [settings,proc]\n");
}
void printf_help_clean(){
    printf("Clean mproc.log file, leaving only currently running processes.\n");
}
void printf_help_update(){
    printf("Get status update from mribg.\n");
}
void printf_help_send(){

    printf("Send direct command to mribg. "
            "This will be processed by process_request. "
            "The arument must be a comma separated string conforming to inputs "
            "accepted by process_request function in mribg.\n"
            "Usage example: send mricom,start,blockstim,design,test\n"
            "");
}


