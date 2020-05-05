/*
 * Analog acquisition test with a comedi command
 *
 */

#include "analogdaq.h"

#define DIGITALTRIG 1
#define VERBOSE 1
#define ANALOGDAQ_TESTING 1

int main(int argc, char **argv){


    // settings and whatnot
    struct ai_settings *as; //analog input settings
    struct dev_settings *ds; // device settings
    struct gen_settings *gs; //general settings

    // files
    char conf[128] = {0};
    char datafile[128] = {0};
    char metafile[128] = {0};
    FILE *fp;
    FILE *fp_meta;

    // timing, log
    struct times *t;
    struct timeval tv;
    struct header *h; // common header for subprocesses
    struct mpid *mp;    // process id struct
    // device and command setup
    comedi_t *dev;
    comedi_cmd c, *cmd=&c;
    int subdev;
    int chanlist[NAICHAN];
    comedi_range *range_info[NAICHAN];
    lsampl_t maxdata[NAICHAN];
    int aref;
    int naichan = NAICHAN;
    int range = 0;
    int period_ns; // sampling period in nanosec, calc from sampling rate

    // memory map device buffer
    void *map;

    // data readout
    unsigned int bufsize;
    unsigned int front, back;
    unsigned int bufpos;
    unsigned int subdev_flags;
    unsigned int sample_size;
    unsigned int nsamples;
    unsigned int col;

    unsigned int i;
    int ret;

    signal(SIGINT, sighandler);
    comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);
    gettimeofday(&tv, NULL);

    //as = malloc(sizeof(struct ai_settings));
    ds = malloc(sizeof(struct dev_settings));
    gs = malloc(sizeof(struct gen_settings));
    h = malloc(sizeof(struct header));
    mp = malloc(sizeof(struct mpid));
    as = malloc(sizeof(struct ai_settings));
    t = malloc(sizeof(struct times));
    for(i=0; i< naichan; i++){
        as->range_info[i]= malloc(sizeof(*range_info));
    }
    parse_gen_settings(gs);
    parse_dev_settings(ds);
    fill_mpid(mp);
    t->start = tv;

    subdev = ds->analog_in_subdev;
    as->period_ns = (int)(1e9 / ds->analog_sampling_rate);
    for(i=0; i< naichan; i++){
        chanlist[i] = ds->analog_in_chan[i];
    }
    if(ds->is_analog_differential==1){
        aref = AREF_DIFF;
    } else {
        fprintf(stderr,"analogdaq: check wiring before setting aref\nExiting.\n");
        exit(1);
    }
    // open device
    dev = comedi_open(ds->devpath); 
    if(dev == NULL){
        comedi_perror("analogdaq: comedi_open");
        exit(1);
    }
    // check if subdev is correct
    ret = comedi_get_subdevice_flags(dev, subdev);
	if(ret < 0 || !(ret & SDF_CMD_READ)) {
		fprintf(stderr, "subdevice %d does not support 'read' commands\n",subdev);
		exit(1);
	}

    // fill ai_settings
    // set up channel list
    for(i = 0; i < naichan; i++){
		as->chanlist[i] = CR_PACK(chanlist[i], range, aref);
		as->range_info[i] = comedi_get_range(dev, subdev, chanlist[i], range);
		as->maxdata[i] = comedi_get_maxdata(dev, subdev, chanlist[i]);
	}
    as->subdev = subdev;
    as->naichan = NAICHAN;
    as->aref = aref;
    if(ANALOGDAQ_TESTING ==1)
        as->n_scan = 1000;

    // prepare metadata file
    

    ret = comedi_get_buffer_size(dev, as->subdev);
    if(ret < 0){
        comedi_perror("comedi_get_bufer_size");
        exit(1);
    }
    bufsize = ret;
    //TODO save these to metadata file as well
    if(VERBOSE==1)
        fprintf(stderr,"bufsize = %d\n",bufsize);
    ret = comedi_get_subdevice_flags(dev, as->subdev);
    if(ret < 0){
		comedi_perror("comedi_get_subdevice_flags");
	}
	subdev_flags = ret;
	sample_size = (subdev_flags & SDF_LSAMPL)
		? sizeof(lsampl_t) : sizeof(sampl_t);
    if(VERBOSE==1)
	    fprintf(stderr,"sample size is %u\n", sample_size);

    //check read subdevice
    ret = comedi_get_read_subdevice(dev);
    if(ret < 0 || ret != as->subdev){
        fprintf(stderr, "failed to setup subdevice\n");
        exit(1);
    }

    // memory map
    map = mmap(NULL, bufsize,PROT_READ,MAP_SHARED,comedi_fileno(dev),0);
    if(VERBOSE==1)
        fprintf(stderr, "map=%p\n", map);
	if( map == MAP_FAILED ){
		perror( "mmap" );
		exit(1);
	}

    // prepare command
    //ret = prepare_cmd_lib(dev, cmd, as);
    ret = prepare_cmd(dev, cmd, as);
    if(ret < 0){
        fprintf(stderr,"prepare_cmd error\n");
        exit(1);
    }

    ret = comedi_command_test(dev, cmd);

    ret = comedi_command_test(dev, cmd);

    if(VERBOSE==1){
        dump_cmd(stdout,cmd);
        print_ai_settings(as);
    }
    if(ret < 0){
        fprintf(stderr, "error: comedi_command_test failed\n");
        exit(1);
    }

    // launch command
    gettimeofday(&tv,NULL);
    t->action = tv;
    ret = comedi_command(dev, cmd);
    if(ret < 0){
        comedi_perror("comedi_command");
        exit(1);
    }
    // data readout
    front = 0;
    back = 0;
    bufpos = 0;
    col = 0;
    while(1){
        ret = comedi_get_buffer_contents(dev,as->subdev);
        if(ret < 0){
            comedi_perror("comedi_get_buffer_contents");
            break;
        }
        front += ret;
        nsamples = (front - back) / sample_size;
        front = back + nsamples * sample_size;
        if(VERBOSE > 1){
            fprintf(stderr, "front = %u, back = %u, samples = %u\n",
                    front, back, nsamples);
        }
        if(front == back){
            comedi_poll(dev, as->subdev);
            usleep(10000);
            continue;
        }
        for(i=0; i < nsamples; i++){
            unsigned int sample;
            if(sample_size == sizeof(sampl_t))
                sample = *(sampl_t *)((char *)map + bufpos);
            else
                sample = *(lsampl_t *)((char *)map + bufpos);
            if(VERBOSE > 0){
                printf("%lf ",to_physical(sample, as, col));
            }
            col++;
            if(col == as->naichan){
                printf("\n");
                col = 0;
            }
            bufpos += sample_size;
            if(bufpos >= bufsize){
                bufpos = 0;
            }
        }

        ret = comedi_mark_buffer_read(dev, as->subdev, front - back);
        if(ret < 0){
            comedi_perror("comedi_mark_buffer_read");
            break;
        }
        back = front;
    }

    // finish
    free(ds);
    free(gs);
    free(h);
    free(mp);
    free(t);
    for(i=0; i<naichan;i++){
        free(as->range_info[i]);
    }
    free(as);

    return 0;
}

/*
 * Function: prepare_cmd
 * ---------------------
 *  prepare command for analog acquisition
 */
int prepare_cmd(comedi_t *dev, comedi_cmd *cmd, struct ai_settings *as){

    memset(cmd,0,sizeof(*cmd));

    cmd->subdev = as->subdev;

    cmd->flags = TRIG_WAKE_EOS;
    /* Wake up at the end of every scan */
	//cmd->flags |= TRIG_WAKE_EOS;

	/* Use a real-time interrupt, if available */
	//cmd->flags |= TRIG_RT;

	/* each event requires a trigger, which is specified
	   by a source and an argument.  For example, to specify
	   an external digital line 3 as a source, you would use
	   src=TRIG_EXT and arg=3. */

    cmd->start_src = TRIG_NOW;
    cmd->start_arg = 0;
	/* The start of acquisition is controlled by start_src.
	 * TRIG_NOW:     The start_src event occurs start_arg nanoseconds
	 *               after comedi_command() is called.  Currently,
	 *               only start_arg=0 is supported.
	 * TRIG_FOLLOW:  (For an output device.)  The start_src event occurs
	 *               when data is written to the buffer.
	 * TRIG_EXT:     start event occurs when an external trigger
	 *               signal occurs, e.g., a rising edge of a digital
	 *               line.  start_arg chooses the particular digital
	 *               line.
	 * TRIG_INT:     start event occurs on a Comedi internal signal,
	 *               which is typically caused by an INSN_TRIG
	 *               instruction.
	 */

    cmd->scan_begin_src = TRIG_TIMER;
    cmd->scan_begin_arg = as->period_ns;
    /* The timing of the beginning of each scan is controlled by
	 * scan_begin.
	 * TRIG_TIMER:   scan_begin events occur periodically.
	 *               The time between scan_begin events is
	 *               convert_arg nanoseconds.
	 * TRIG_EXT:     scan_begin events occur when an external trigger
	 *               signal occurs, e.g., a rising edge of a digital
	 *               line.  scan_begin_arg chooses the particular digital
	 *               line.
	 * TRIG_FOLLOW:  scan_begin events occur immediately after a scan_end
	 *               event occurs.
	 * The scan_begin_arg that we use here may not be supported exactly
	 * by the device, but it will be adjusted to the nearest supported
	 * value by comedi_command_test(). */

    cmd->convert_src = TRIG_TIMER;
    cmd->convert_arg = 1;
	/* The timing between each sample in a scan is controlled by convert.
	 * TRIG_TIMER:   Conversion events occur periodically.
	 *               The time between convert events is
	 *               convert_arg nanoseconds.
	 * TRIG_EXT:     Conversion events occur when an external trigger
	 *               signal occurs, e.g., a rising edge of a digital
	 *               line.  convert_arg chooses the particular digital
	 *               line.
	 * TRIG_NOW:     All conversion events in a scan occur simultaneously.
	 * Even though it is invalid, we specify 1 ns here.  It will be
	 * adjusted later to a valid value by comedi_command_test() */

    cmd->scan_end_src = TRIG_COUNT;
    cmd->scan_end_arg = as->naichan;
	/* The end of each scan is almost always specified using
	 * TRIG_COUNT, with the argument being the same as the
	 * number of channels in the chanlist.  You could probably
	 * find a device that allows something else, but it would
	 * be strange. */

    cmd->stop_src = TRIG_NONE;
    cmd->stop_arg = 0;
	/* The end of acquisition is controlled by stop_src and
	 * stop_arg.
	 * TRIG_COUNT:  stop acquisition after stop_arg scans.
	 * TRIG_NONE:   continuous acquisition, until stopped using
	 *              comedi_cancel()
	 * */
    
    cmd->chanlist = as->chanlist;
    cmd->chanlist_len = as->naichan;

    return 0;
}

/*
 * Function: prepare_cmd_lib
 * -------------------------
 *  preapare generic command only for test purposes
 */
int prepare_cmd_lib(comedi_t *dev, comedi_cmd *cmd, struct ai_settings *as){

    int ret;
    ret = comedi_get_cmd_generic_timed(
            dev, as->subdev, cmd, as->naichan, as->period_ns);
    if(ret < 0){
        comedi_perror("comedi_get_cmd_generic_timed\n");
        return ret;
    }
    cmd->chanlist = as->chanlist;
    cmd->chanlist_len = as->naichan;
    if(cmd->stop_src == TRIG_COUNT)
        cmd->stop_arg = as->n_scan;
    return 0;
}

void dump_cmd(FILE *out,comedi_cmd *cmd){

	char buf[100];

	fprintf(out,"subdevice:      %d\n",
		cmd->subdev);

	fprintf(out,"start:      %-8s %d\n",
		cmd_src(cmd->start_src,buf),
		cmd->start_arg);

	fprintf(out,"scan_begin: %-8s %d\n",
		cmd_src(cmd->scan_begin_src,buf),
		cmd->scan_begin_arg);

	fprintf(out,"convert:    %-8s %d\n",
		cmd_src(cmd->convert_src,buf),
		cmd->convert_arg);

	fprintf(out,"scan_end:   %-8s %d\n",
		cmd_src(cmd->scan_end_src,buf),
		cmd->scan_end_arg);

	fprintf(out,"stop:       %-8s %d\n",
		cmd_src(cmd->stop_src,buf),
		cmd->stop_arg);
}

char *cmd_src(int src,char *buf)
{
	buf[0]=0;

	if(src&TRIG_NONE)strcat(buf,"none|");
	if(src&TRIG_NOW)strcat(buf,"now|");
	if(src&TRIG_FOLLOW)strcat(buf, "follow|");
	if(src&TRIG_TIME)strcat(buf, "time|");
	if(src&TRIG_TIMER)strcat(buf, "timer|");
	if(src&TRIG_COUNT)strcat(buf, "count|");
	if(src&TRIG_EXT)strcat(buf, "ext|");
	if(src&TRIG_INT)strcat(buf, "int|");
#ifdef TRIG_OTHER
	if(src&TRIG_OTHER)strcat(buf, "other|");
#endif

	if(strlen(buf)==0){
		sprintf(buf,"unknown(0x%08x)",src);
	}else{
		buf[strlen(buf)-1]=0;
	}

	return buf;
}

void print_ai_settings(struct ai_settings *as){

    int i;
    printf("\nstruct ai_settings:\n");
    printf("-------------------\n");
    printf("subdev: %d\n",as->subdev);
    printf("naichan: %d\n",as->naichan);
    printf("aref: %d\n",as->aref);
    printf("chanlist: ");
    for(i=0;i<as->naichan;i++)
        printf("%d,",as->chanlist[i]);
    printf("\nn_scan: %d\n",as->n_scan);
    printf("period_ns: %d\n",as->period_ns);


}

double to_physical(unsigned int sample, struct ai_settings *as, int ch){

    double phys;

    phys = comedi_to_phys(sample, as->range_info[ch], as->maxdata[ch]);
    return phys;

}
