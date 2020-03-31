/* parser.c
 * 
 * parse vnmrj procpar file
 *
 * parse settings file
 */

#include "mricom.h"

int parse_procpar(){

    char procpar[] = "/home/david/mricom/test/procpar";
    char line[128];
    char cmd[32];
    FILE *fp;
    int i = 0;
    fp = fopen(procpar,"r");
    if(fp == NULL){
        print("\nerror opening %s\n",procpar);
        printf("returning ...\n");
        return 1;
    }
    while(fgets(line, 128, fp)){
        i++;
        printf()
    }

}

int parse_settings(){

}


