/* comedifunc.c
 *
 * Contains functions using the comedi library
 */

#include "mricom.h"

extern daq_settings *settings;
extern dev_settings *devsettings;

static unsigned int chanlist[NAICHAN];
static comedi_range * range_info[NAICHAN];
static lsampl_t maxdata[NAICHAN];

void print_datum(lsampl_t raw, int channel_index);

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
 * Function: comedi_start_analog_acq:
 * ------------------------------------------
 * setup analog input subdevice, command, then start
 *
 * Uses hard-defined parameters in settings and mricom.h
 */
 int comedi_start_analog_acq(){

    comedi_t *dev;
    comedi_cmd *cmd;

    int subdev_flags;
    int aref;                       // int indicating analog ref type
    int chan;
    int subdev;
    int chanlist[NAICHAN];
    int n_chan = NAICHAN;           // number of acquisition channels
    int range = 0;                  //??
    int n_scan = 1000;              // number of scans, TODO this is testing
    double freq = 10.0;             // 

    char buf[BUFSZ];
    double val;
    int scan_period_ns;
    int i, ret, col;
    int total = 0;
    lsampl_t raw;

    comedi_range *range_info[NAICHAN];
    lsampl_t maxdata[NAICHAN];

    dev = devsettings->dev;
    cmd = devsettings->cmd;

    comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

    if(dev == NULL){
        comedi_perror(settings->device);
        exit(1);
    }
    subdev = devsettings->analog_in_subdev;

    // check if subdev is correct
    ret = comedi_find_subdevice_by_type(dev, COMEDI_SUBD_AI, 0);

    if(ret != subdev){
        printf(" Settings specified subdevice is not analog input subdevice\n");
        printf(" Exiting...\n");
        exit(1);
    }
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
    if(DEBUG >1){
        printf(" daq setup: subdev, nchan: %d, %d\n",subdev,n_chan);
    }
    ret = comedi_get_cmd_generic_timed(dev, subdev, cmd, n_chan, 1e9);
    if(ret < 0){
        comedi_perror("comedi_get_cmd_generic_timed");
        exit(1);
    }

    cmd->start_src = TRIG_NOW;
    cmd->chanlist = chanlist;
    cmd->chanlist_len = n_chan;
    //TODO del this after testing
    cmd->stop_src = TRIG_COUNT;
    cmd->stop_arg = n_scan;

    // test if everything is ok
    ret = comedi_command_test(dev, cmd);
    if(DEBUG > 0 && ret == 0){
        printf(" comedi_command_test ok\n");
    }
    if(ret < 0){
        comedi_perror("comedi_command_test");
        exit(1);
    }
    if(ret != 0){
        printf("comedi_setup_analog_acq: error preparing command: %d\n",ret);
        exit(1);
    }

    // run command
    ret = comedi_command(dev, cmd);
    if(ret < 0){
        comedi_perror("comedi_command");
        exit(1);
    }
    printf("analog command running\n");
    col = 0;
    subdev_flags = comedi_get_subdevice_flags(dev, subdev);
    while(1){
        printf("yaaay\n");
        ret = read(comedi_fileno(dev),buf, BUFSZ);
        printf("ret %d\n",ret);
        if(ret < 0){
            perror("read");
            break;
        } else if (ret == 0){
            // stop condition
            break;
        } else {
            int bytes_per_sample;
            total += ret;

            if(subdev_flags & SDF_LSAMPL){
                bytes_per_sample = sizeof(lsampl_t);
            } else {
                bytes_per_sample = sizeof(sampl_t);
            }
            for (i = 0; i < ret / bytes_per_sample; i++){
                if (subdev_flags & SDF_LSAMPL) {
                    raw = ((lsampl_t *)buf)[i];
                } else {
                    raw = ((sampl_t *)buf)[i];
                }
                print_datum(raw, col);
                col++;
                if(col == n_chan){
                    printf("\n");
                    col = 0;
                }
            }
        }
    }
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
/*
 * Function: print_datum
 * ---------------------
 * utility to print analog acq data
 */

void print_datum(lsampl_t raw, int channel_index){

	double physical_value;

	physical_value = comedi_to_phys(raw, range_info[channel_index],
					maxdata[channel_index]);
	printf("%#8.6g ", physical_value);
}

