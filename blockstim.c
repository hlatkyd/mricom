#include "blockstim.h"

#define BLOCKSTIM_TESTING 1
#define LOG_TTL_LEADING 1   // log the rising edge of TTL
#define LOG_SEC 1           // write log file in sec format

/* This is a digital triggering solution without addititive timing error
 * and microsec resolution.
 *
 * Usage:
 * Fill blockstim.conf file with appropriate parameters for a desired block
 * design. then call "./blockstim design [x]" from command line
 */


int main(int argc, char *argv[]){
    /* block:
     *          ttl_width
     *          _________       ________          
     *          |       |       |       |        
     *          |       |       |       |
     *          |       |       |       |
     * _________|       |_______|       |________
     *
     *          [   1 / frq     ]                    
     *
     *
     * run: start_delay,(block) x n_cycles, end_delay
     *
     * ARGS:
     *      1 - ?? TODO
     */



    comedi_t *dev;
    char devpath[] = "/dev/comedi0";
    char conf[] = "blockstim.conf";     // config file, parse for defaults
    char logfile[] = "blockstim.tsv";   // tab separated data file
    char metafile[] = "blockstim.meta"; // metadata for tsv file
    FILE *fp;
    char parent_name[32];
    int is_mricom_child = 0; // set to 1 later if true
    struct header_common *header;

    char design[16];
    struct blockstim_settings *bs;

    int usec_ttl1;                      // time while TTL is on in microsec
    int usec_ttl0;                      // time while TTL is off
    int duration;                       // full duration in seconds

    struct timeval start_tv, stop_tv;   // calc elapsed time
    double diff;
    struct timeval tv;                  // for TTL timing
    struct timespec nano;               // for wait with nanosleep
    nano.tv_sec = 0;                    //
    nano.tv_nsec = 1;                   //
    int usec_temp;                      // for logging
    int usec, usec_start, usec_target;  // measure, set target times

    int i,j, n;
    int ret;
    int TEST = 0;                       // indicate test stim, no saving
    
    header = malloc(sizeof(struct header_common));
    gettimeofday(&start_tv,NULL);
    header->start_time = tv;
    strcpy(header->process_name, argv[0]);
    // check if parent is mricom
    getppname(parent_name);
    if(strcmp(parent_name, "mricom") == 0){
        is_mricom_child = 1;
        //TODO local pid control here
    }
    bs = malloc(sizeof(struct blockstim_settings));

    if (argc == 3 && strcmp(argv[1], "design") == 0){
        strcpy(design,argv[2]);
        parse_blockstim_conf(bs, conf, design);
        if(strncmp(argv[2], "test", 4) == 0){
            TEST = 1; // set to not save csv and log
        }
    } else {
        // default to design 1
        parse_blockstim_conf(bs, conf, "default");
    }

    usec_ttl1 = bs->ttl_usecw;
    usec_ttl0 = (int)(1.0 / bs->ttl_freq * 1000000) - usec_ttl1;
    n = (int) ((bs->on_time * 1e6) / (usec_ttl1+usec_ttl0));

    // only for testing
    if(BLOCKSTIM_TESTING > 0){
        printf("subdev %d\n",bs->subdev);
        printf("chan %d\n",bs->chan);
        printf("start_delay %lf\n",bs->start_delay);
        printf("on_time %lf\n",bs->on_time);
        printf("off_time %lf\n",bs->off_time);
        printf("ttl_usecw %d\n",bs->ttl_usecw);
        printf("ttl_freq %lf\n",bs->ttl_freq);
        printf("n_blocks %d\n",bs->n_blocks);
        printf("usec ttl0 : %d\n",usec_ttl0);
        printf("n in an ON block: %d\n",n);
        //printf("press any key\n");
        //getchar();
    }
    // wrinting out timing to file
    if(TEST != 1){
        fp = fopen(logfile,"w");
        if(fp == NULL){
            perror("fopen");
            exit(1);
        }
        // prepare header
        prepare_log(fp, parent_name, bs);
    }
    // device setup
    dev = comedi_open(devpath);
    if(dev == NULL){
        comedi_perror("comedi_open");
    }
    // check if subdev is correct
    ret = comedi_get_subdevice_type(dev, bs->subdev);
    if(ret != COMEDI_SUBD_DIO){
        printf("wrong subdev");
        exit(1);
    }
    comedi_dio_config(dev, bs->subdev, bs->chan, COMEDI_OUTPUT);

    usec_start = 0;
    gettimeofday(&tv,NULL);
    diff = getsecdiff(start_tv, tv);
    printf("setup time: %lf\n",diff);
    usec_target = usec_start + (int)(bs->start_delay * 1000000.0);
    printf("starting...\n");
    for(j=0; j < bs->n_blocks; j++){
        // this is an ON block
        for(i=0; i < n; i++){
            // wait for usec target reach and then set TTL ON
            do {
                usec = getusecdelay(tv);
                nanosleep(&nano, NULL);
            } while(usec < usec_target);
            usec_target += usec_ttl1;
            comedi_dio_write(dev, bs->subdev, bs->chan, 1);
            // wait for usec target and then set TTL OFF
            do{
                usec = getusecdelay(tv);
                nanosleep(&nano, NULL);
            } while(usec < usec_target);
            usec_target += usec_ttl0;
            comedi_dio_write(dev, bs->subdev, bs->chan, 0);

            // log TTL time values
            if(TEST != 1){
                usec_temp = getusecdelay(tv);
                append_log(fp,i,usec_temp, usec_ttl1);
            }
        }
        // OFF block, set usec target
        usec_target += (int)(bs->off_time * 1000000.0);

    }
    gettimeofday(&stop_tv, NULL);
    diff = getsecdiff(start_tv, stop_tv);
    printf("elapsed time: %lf\n",diff);
    if(TEST != 1){
        fclose(fp);
    }
    fp = fopen(metafile,"w");
    if(fp == NULL){
        printf("blockstim: cannot open %s\n",metafile);
        exit(1);
    }
    fprintf_header_common(fp, header);
    // close tsv, write meta
    comedi_close(dev);
}
/*
 * Function: getusecdelay
 * ----------------------
 * Calculate current difference in microsec from input time
 */
int getusecdelay(struct timeval tv1){

    struct timeval tv2;
    int time;
    int mic;
    double mega = 1000000;
    gettimeofday(&tv2,NULL);
    time = (tv2.tv_sec - tv1.tv_sec) * mega;
    mic = (tv2.tv_usec - tv1.tv_usec);
    mic += time;
    return mic;
}
/*
 * Function: getsecdiff
 * ----------------------
 * Calculate difference in seconds (double) between two timepoints
 */
double getsecdiff(struct timeval tv1, struct timeval tv2){

    double diff;
    diff = tv2.tv_sec - tv1.tv_sec;
    diff += (double) (tv2.tv_usec - tv1.tv_usec) / 1000000.0;
    return diff;
}
/*
 * Function: prepare_log
 * ---------------------
 * Prepare log header, leave blank space where data is acquired later
 */
void prepare_log(FILE *fp, char *parent, struct blockstim_settings *bs){


}

void append_log(FILE *fp, int n, int time, int usec_ttl1){

    if(LOG_TTL_LEADING == 1){
        time -= usec_ttl1;
    }
    fprintf(fp,"%d,%d\n",n,time);

}
/*
 * Function: finish_log_header
 * ---------------------------
 * Fill in elapsed times
 */
void finish_log_header(FILE *fp){

}
