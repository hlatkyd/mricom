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

    printf("Print available commands or get help for a command.\n"
           "Usage: help [command name]\n");
}
void printf_help_killp(){
    printf("Force kill an mricom internal process.\n"
            "Type 'list proc' to see the running processes and their IDs.\n"
            "Usage: killp [process ID]\n"
            );
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
    printf("List stuff: settings, parameters, processes.\n"
            "Currently accepted as argument:\n"
            "   settings     print general and device settings\n"
            "   proc         print currently running mricom processes\n"
            "   stury        print currently running study elements\n"
            "   anesth       print anesthesia logs\n"
            "Usage:\n"
            "list [arg]\n"
            );
}
void printf_help_clean(){
    printf("Clean mproc.log file, leaving only currently running processes.\n");
}
void printf_help_update(){
    printf("Get status update from mribg.\n");
}
void printf_help_send(){

    printf("Send direct command to mribg. This function is only intended\n"
            "for developmental use.\n"
            "The command will be processed by the function process_request.\n"
            "It should be a comma separated string conforming to\n"
            "inputs accepted by process_request function in mribg.\n"
            "See mribg.c for more information.\n\n"
            "Usage example:\n"
            "send mricom,start,blockstim,design,test\n"
            "");
}

void printf_help_set(){

    printf("Set parameters for mribg. Available parameters are:\n"
            "   status  ????\n"
            "   iso     isoflurane percentage, used for anesthesia logging\n"
            "   id      study id, example: s_2020080401\n"
            "Usage:\n"
            "set [parameter name] [value]\n"
          );
}

void printf_help_get(){

}
