/* comedifunc.c
 *
 * Contains functions using the comedi library
 */

#include "mricom.h"

extern daq_settings *settings;
extern dev_settings *devsettings;

/*
 * Function: comedi_device_setup
 * -----------------------------
 * open comedi device
 *
 */

int comedi_device_setup(){

    comedi_t *device;
    device = comedi_open(settings->device);
    if(device == NULL){
        printf("\nerror: could not open comedi device");
        return 1;
    }
    printf("comedi device %s open.\n",settings->device);
    devsettings->dev = device;
    return 0;
}
/*
 * Fucntion: comedi_device_close
 * -----------------------------
 * close previously opened comedi device 
 */
int comedi_device_close(){

    int ret;
    ret = comedi_close(devsettings->dev);
    if(ret == 0){
        printf("closing comedi device %s...\n",settings->device);
        return 0;
    } else {
        printf("could not close comedi device %s\n",settings->device);
        return 1;
    }
}


/*
 * Function: comedi_digital_trig
 * -----------------------------
 * send a TTL signal to digital out
 */

int comedi_digital_trig(char *eventfile){

    unsigned int subdev = 2;
    unsigned int chan = 0;
    int range = 0;
    int aref = AREF_GROUND;
    comedi_t *device;
    int retval;
    unsigned int bit;
    int i;

    device = devsettings->dev;

    retval = comedi_dio_config(device, subdev, chan, COMEDI_OUTPUT);
    if(retval == -1){
        printf("there's an error mate\n");
    }
    for(i=0;i<5;i++){
        bit = 1;
        retval = comedi_dio_write(device,subdev,chan, bit);
        if(retval == -1){
            printf("there's an error mate\n");
        }
        sleep(1);
        bit = 0;
        retval = comedi_dio_write(device,subdev,chan, bit);
        if(retval == -1){
            printf("there's an error mate\n");
        }
        sleep(1);
    }


    return 0;
}

/*
 * Function: comedi_setup_analog_acq:
 * ------------------------------------------
 * read event file for digital trigger generation
 */
int comedi_setup_analog_acq(){

    return 0;
}

/*
 * Function: comedi_start_analog_acq:
 * ------------------------------------------
 * read event file for digital trigger generation
 */
int comedi_start_analog_acq(){

    comedi_t *device;

    device = devsettings->dev;
    return 0;

}

/*
 * Function: comedi_setup_digital_sequence
 * ------------------------------------------
 * read event file for digital trigger generation
 */
//TODO
// set up instruction list
int comedi_setup_digital_sequence(char *filename){

}
/*
 * Function: comedi_execute_digital_sequence
 * ------------------------------------------
 * execute digital triggering as per time_buffer data
 */
//TODO
// execute instruction list
// test timing
int comedi_execute_digital_sequence(){


}
