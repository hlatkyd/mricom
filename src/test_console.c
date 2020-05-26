/*
 * mockconsole
 *
 * Subprogram only for testing. Simulates console user output to test ttlctrl.
 *
 */

#include "common.h"
#include "socketcomm.h"

#define OPTION 1
int main(int argc, char **argv){

    struct gen_settings *gs;
    struct dev_settings *ds;
    struct mpid *mp;
 
    comedi_t *dev;
    char mricomdir[LPATH];
    int subdev, outchan, inchan;
    int i, ret;
    strcpy(mricomdir, getenv("MRICOMDIR"));

    gs = malloc(sizeof(struct gen_settings));
    ds = malloc(sizeof(struct dev_settings));

    parse_gen_settings(gs);
    parse_dev_settings(ds);

    subdev = ds->test_console_subdev;
    inchan  = ds->test_console_in_chan;
    outchan = ds->test_console_out_chan;

    comedi_dio_config(dev, subdev, inchan, COMEDI_INPUT);
    comedi_dio_config(dev, subdev, outchan, COMEDI_OUTPUT);
    comedi_dio_write(dev, subdev, outchan , 0);


    return 0;

}

