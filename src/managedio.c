/*
 * managedio.c
 *
 * Set values on digital channels, calibrate device, etc.
 * Usable with PCI-6035E so far
 *
 * Calling:
 *  ./managedio zero:
 * Reset digital channels to output 0V
 *  ./managedio calibrate
 *  TODO
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <comedilib.h>

#define SUBDEV 2
#define N_DIO 8
#define DEVPATH "/dev/comedi0"
int main(int argc, char **argv){

    int chanlist[N_DIO];
    int subdev = SUBDEV;
    int i, ret;
    comedi_t *dev;
    dev = comedi_open(DEVPATH);
    // print some help
    if(argc == 1 || strcmp(argv[1],"--help")==0 || strcmp(argv[1],"-h")==0){
        printf(" Set values to digital channels.\n"
                "Usage:\n"
                "$ managedio zero       Set 0 V on digital channels.\n"
                "$ managedio calibrate  TODO\n"
                );
        return 0;
    }

    
    if(argc != 2){
        fprintf(stderr, "Please specify input argument\n");
        exit(1);
    }
    // checking whether defines are correct with device
    ret = comedi_get_subdevice_type(dev, subdev);
    if(ret != COMEDI_SUBD_DIO){
        fprintf(stderr, "uncorrect subdev given, searching for DIO subdev\n");
        subdev = comedi_find_subdevice_by_type(dev, COMEDI_SUBD_DIO, 0);
        if(subdev == -1){
            fprintf(stderr, "Cannot find DIO subdev. Exiting.\n");
            exit(1);
        }
        fprintf(stderr, "Found DIO subdev: %d\n",subdev);
    }
    //comedi_get_n_channels
    if(dev == NULL){
        comedi_perror("comedi_open");
        exit(1);
    }
    // on zero:
    if(strcmp(argv[1],"zero")==0){
        for(i=0; i<N_DIO; i++){
            chanlist[i] = i;
        }

        for(i=0; i<N_DIO; i++){
            comedi_dio_config(dev, subdev, chanlist[i], COMEDI_OUTPUT);
            ret = comedi_dio_write(dev, subdev, chanlist[i] , 0);
            if(ret < 0){
                comedi_perror("comedi_dio_write");
                exit(1);
            }
            printf(" Channel %d on subdevice %d set to zero.\n",chanlist[i],subdev);
        }
    }
    // TODO on calirate
    else{
        fprintf(stderr, "Argument not recognized, exiting.\n");
    }
    return 0;
}
