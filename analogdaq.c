/*
 * Analog acquisition test with a comedi command
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <comedilib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define DIGITALTRIG 1

#define NCHAN 3
#define BUFSZ 1000
#define VERBOSE 1
char buf[BUFSZ]; // TODO what is this exaclty?

unsigned int subdev = 0;
unsigned int chan = 0; // channels start from here, so: 0,1,2
unsigned int n_chan = NCHAN; // number of analog input channels 
char devfile[] = "/dev/comedi0";
unsigned int p_ns = (unsigned int) 1e6; // scan period in nanosec

static unsigned int chanlist[NCHAN];
static comedi_range *range_info[NCHAN];
static lsampl_t maxdata[NCHAN];

void print_datum(sampl_t raw, int channel_index);
int set_cmd_params(comedi_cmd *cmd, unsigned int p_ns, unsigned int n_scans);

int main(int argc, char **argv){

    comedi_t *dev;
    comedi_cmd c, *cmd=&c;
    comedi_insn insn;
    unsigned int aref = AREF_DIFF;
    unsigned int range = 0; //TODO what is this again??
    struct timeval start, end;
    sampl_t raw; // 16bit sampl_t type is enough on PCI-6035E analog input subdev

    int n_scans = 1000000; // how many points to sample?
    int ret;
    int total = 0;
    int i;

    dev = comedi_open(devfile);
    if(dev == NULL){
        comedi_perror("comedi_open");
    }
    
    // check if settings are ok
    ret = comedi_get_subdevice_flags(dev, subdev);
	if(ret < 0 || !(ret & SDF_CMD_READ)) {
		fprintf(stderr, "subdevice %d does not support 'read' commands\n",subdev);
		exit(1);
	}
    // set up channel list
    for(i = 0; i < n_chan; i++){
		chanlist[i] = CR_PACK(chan + i, range, aref);
		range_info[i] = comedi_get_range(dev, subdev, chan, range);
		maxdata[i] = comedi_get_maxdata(dev, subdev, chan);
	}

    // set up command
    comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);
    memset(cmd, 0, sizeof(*cmd));

    ret = comedi_get_cmd_generic_timed(dev, subdev, cmd, n_chan, p_ns);
    if(ret < 0){
        comedi_perror("comedi_cmd_get_generic_timed");
    }

    set_cmd_params(cmd, p_ns, n_scans);
    ret = comedi_command_test(dev, cmd);
    if(ret < 0){
        comedi_perror("comedi_command_test");
    }
    if(VERBOSE == 1) printf("command tested OK\n");
    // set up internal trigger instruction
    memset(&insn, 0, sizeof(comedi_insn));
    lsampl_t data[1];
    data[0] = 0;
    insn.insn = INSN_INTTRIG;
    insn.subdev = subdev;
    insn.data = data;
    insn.n = 1;

    // arm subdevice for data collection
    ret = comedi_command(dev, cmd);
    if(ret < 0){
        comedi_perror("comedi_command");
        exit(1);
    }
    if(VERBOSE == 1) printf("command armed on subdevice %d\n",subdev);
    //TODO delet this later
    printf("Press Any Key to Continue\n");
    getchar();
    ret = comedi_do_insn(dev,&insn);
    if(ret < 0){
        comedi_perror("comedi_do_insn");
        exit(1);
    }
    gettimeofday(&start, NULL);
    // TODO internal trigger here


    // statrt data collection loop
    int row = 0;
    while(1){
		ret = read(comedi_fileno(dev),buf,BUFSZ);
		if(ret < 0){
			/* some error occurred */
			perror("read");
			break;
		}else if(ret == 0){
			/* reached stop condition */
			break;
		}else{
			static int col = 0;
			int bytes_per_sample;
			//total += ret;
			//fprintf(stderr, "read %d %d\n", ret, total);
            bytes_per_sample = sizeof(sampl_t);
			for(i = 0; i < ret / bytes_per_sample; i++){
                raw = ((sampl_t *)buf)[i];
				print_datum(raw, col);
				col++;
				if(col == n_chan){
					printf("\n%d ",row);
					col=0;
                    row++;
				}
			}
		}
	}
    gettimeofday(&end,NULL);
    end.tv_sec -= start.tv_sec;
    if(end.tv_usec < start.tv_usec){
        end.tv_sec--;
        end.tv_usec += 1000000;
    }
    end.tv_usec -= start.tv_usec;
    printf("time = %ld.%06ld\n",end.tv_sec,end.tv_usec);
    return 0;

}

void print_datum(sampl_t raw, int channel_index) {
	double physical_value;
    physical_value = comedi_to_phys(raw, range_info[channel_index], maxdata[channel_index]);
    printf("%#8.6g ",physical_value);
}

int set_cmd_params(comedi_cmd *cmd, unsigned int p_ns, unsigned int n_scans){

    cmd->flags = 0;
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
    cmd->start_src = TRIG_INT;
    cmd->start_arg = 0;
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
    cmd->scan_begin_src = TRIG_TIMER;
    cmd->scan_begin_arg = p_ns;
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
     cmd->convert_src = TRIG_TIMER;
     cmd->convert_arg = 1;
     	/* The end of each scan is almost always specified using
	 * TRIG_COUNT, with the argument being the same as the
	 * number of channels in the chanlist.  You could probably
	 * find a device that allows something else, but it would
	 * be strange. */
	cmd->scan_end_src =	TRIG_COUNT;
	cmd->scan_end_arg =	n_chan;		/* number of channels */
	/* The end of acquisition is controlled by stop_src and
	 * stop_arg.
	 * TRIG_COUNT:  stop acquisition after stop_arg scans.
	 * TRIG_NONE:   continuous acquisition, until stopped using
	 *              comedi_cancel()
	 * */
	cmd->stop_src =	TRIG_COUNT;
	cmd->stop_arg =	n_scans;
	/* the channel list determined which channels are sampled.
	   In general, chanlist_len is the same as scan_end_arg.  Most
	   boards require this.  */
	cmd->chanlist =	chanlist;
	cmd->chanlist_len =	n_chan;

    return 0;
}
