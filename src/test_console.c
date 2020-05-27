/*
 * mockconsole
 *
 * Subprogram only for testing. Simulates console user output to test ttlctrl.
 *
 * 
 */

#include "common.h"
#include "socketcomm.h"

#define OPTION 1 // option 1 is handshake

// pulsesequence mock functions 
void xgate(comedi_t *dev, int subdev, int chan, int num);
void spx_on(comedi_t *dev, int subdev, int chan);
void spx_off(comedi_t *dev, int subdev, int chan);

int main(int argc, char **argv){

    struct gen_settings *gs;
    struct dev_settings *ds;
    struct mpid *mp;
 
    comedi_t *dev;
    char mricomdir[LPATH];
    int subdev, outchan, inchan;
    int i, ret;
    int bit = 0;
    strcpy(mricomdir, getenv("MRICOMDIR"));

    gs = malloc(sizeof(struct gen_settings));
    ds = malloc(sizeof(struct dev_settings));

    parse_gen_settings(gs);
    parse_dev_settings(ds);

    dev = comedi_open(ds->devpath);
    if(dev == NULL){
        comedi_perror("comedi_open");
        exit(1);
    }
    subdev = ds->test_console_subdev;
    inchan  = ds->test_console_in_chan;
    outchan = ds->test_console_out_chan;

    comedi_dio_config(dev, subdev, inchan, COMEDI_INPUT);
    comedi_dio_config(dev, subdev, outchan, COMEDI_OUTPUT);
    comedi_dio_write(dev, subdev, outchan , 0);
    
    // simulated sequence start
    if(OPTION == 1){
        // handshake option with ttlctrl
        spx_on(dev, subdev, outchan);
        xgate(dev, subdev, inchan, 1);
        spx_off(dev, subdev, outchan);
        usleep(5);
        spx_on(dev, subdev, outchan);

    }
    // now wait for TTL
    xgate(dev, subdev, inchan,1);
    spx_off(dev, subdev, outchan);

    // simulate end of sequence
    spx_on(dev, subdev, outchan);
    usleep(30);
    spx_off(dev, subdev, outchan);
    return 0;

}
// counts TTLs till num
void xgate(comedi_t *dev, int subdev, int chan, int num){

    int i = 0;
    int bit = 0;
    // check first
    comedi_dio_read(dev, subdev, chan, &bit);
    /*
    if(bit == 1){
        fprintf(stderr, "xgate received TTL on at start\n");
        exit(1);
    }
    */
    fprintf(stderr, "xgate: waiting for input\n");
    for(i=0; i<num; i++){
        while(bit != 1)
            comedi_dio_read(dev, subdev, chan, &bit);
        while(bit != 0)
            comedi_dio_read(dev, subdev, chan, &bit);
    }
    fprintf(stderr, "xgate: got all (%d) input\n",num);
    
}
void spx_on(comedi_t *dev, int subdev, int chan){

    comedi_dio_write(dev, subdev, chan, 1);
}
void spx_off(comedi_t *dev, int subdev, int chan){

    comedi_dio_write(dev, subdev, chan, 0);

}

