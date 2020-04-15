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
 * setup analog input subdevice, command, etc
 *
 * Uses hard-defined parameters in settings and mricom.h and returns 
 * a prepared comedi command type 'comedi_cmd'
 */
comedi_cmd *comedi_setup_analog_acq(){

    comedi_t *dev;
    comedi_cmd c, *cmd = &c;
    int subdev_flags;
    int aref;                       // int indicating analog ref type
    int chan;
    int subdev;
    int chanlist[NAICHAN];
    int n_chan = NAICHAN;           // number of acquisition channels
    int range = 0;                  //??
    int n_scan = 1000;              // number of scans, TODO this is testing
    double freq = 10.0;             // 
    double val;
    int scan_period_ns;
    int ret;
    int i;
    comedi_range *range_info[NAICHAN];
    lsampl_t maxdata[NAICHAN];

    dev = devsettings->dev;

    comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

    if(dev == NULL){
        comedi_perror(settings->device);
        exit(1);
    }
    subdev = devsettings->analog_in_subdev;
    for(i=0;i<NAICHAN;i++){
        chan = devsettings->analog_in_chan[i];
        chanlist[i] = CR_PACK(chan, range, aref);
        range_info[i] = comedi_get_range(dev, subdev, chan, range);
        maxdata[i] = comedi_get_maxdata(dev, subdev, chan);
    }
    if(devsettings->is_analog_differential == 1){
        aref = AREF_DIFF;
    } else {
        printf(" comedi_setup_analog_acq: only diff is tested\n");
        aref = AREF_GROUND;
    }

    // init command
    memset(cmd,0,sizeof(*cmd));
    // make generic command, then modify it
    printf("dev, subdev, nchan: %p, %d, %d\n",dev, subdev,n_chan);
    ret = comedi_get_cmd_generic_timed(dev, subdev, cmd, n_chan, 1e9);
    printf("ret %d\n",ret);
    //cmd->start_src = TRIG_NOW;
    cmd->chanlist = chanlist;
    cmd->chanlist_len = n_chan;
    //TODO del this after testing
    //if(cmd->stop_src == TRIG_COUNT){
    //    cmd->stop_arg = n_scan;
    //}


    // test if everything is ok
    ret = comedi_command_test(dev, cmd);
    printf("ret %d\n",ret);
    if(ret < 0){
        comedi_perror("comedi_command_test");
        exit(1);
    }
    if(ret != 0){
        printf("comedi_setup_analog_acq: error preparing command: %d\n",ret);
        exit(1);
    }

    devsettings->cmd = cmd;
    return cmd;
}

/*
 * Function: comedi_start_analog_acq:
 * ------------------------------------------
 * start analog acquisition command
 */
int comedi_start_analog_acq(comedi_cmd *cmd){

    int ret;
    comedi_t *dev;

    dev = devsettings->dev;

    // start the command
    ret = comedi_command(dev, cmd);
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
